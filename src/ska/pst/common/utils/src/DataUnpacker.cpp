/*
 * Copyright 2023 Square Kilometre Array Observatory
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdexcept>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/DataUnpacker.h"
#include "ska/pst/common/definitions.h"

void ska::pst::common::DataUnpacker::configure(const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config)
{
  layout.configure(data_config, weights_config);

  // extract the required parameters from the data header
  ndim = data_config.get_uint32("NDIM");
  npol = data_config.get_uint32("NPOL");
  nbit = data_config.get_uint32("NBIT");
  nchan = data_config.get_uint32("NCHAN");

  bandpass.resize(nchan);
  for (unsigned ichan=0; ichan<nchan; ichan++)
  {
    bandpass[ichan].resize(npol);
  }
  reset();
}

auto ska::pst::common::DataUnpacker::get_scale_factor(char * weights, uint32_t packet_number) -> float
{
  auto * weights_ptr = reinterpret_cast<float *>(weights + (packet_number * layout.get_weights_packet_stride())); // NOLINT
  // return the scale factor, ignoring invalid value of 0
  if (*weights_ptr == 0) {
    return 1;
  } else {
    return *weights_ptr;
  }
}

void ska::pst::common::DataUnpacker::resize(uint64_t data_bufsz)
{
  // determine the unpacked data size in TFP order
  const uint64_t nsamp = (data_bufsz * ska::pst::common::bits_per_byte) / (nchan * npol * ndim * nbit);
  const uint64_t nval = nsamp * nchan * npol;

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::resize nsamp={} nchan={} npol={} nval={}", nsamp, nchan, npol, nval);
  if (unpacked.size() != nsamp)
  {
    unpacked.resize(nsamp);
  }

  for (unsigned isamp=0; isamp<nsamp; isamp++)
  {
    if (unpacked[isamp].size() != nchan)
    {
      unpacked[isamp].resize(nchan);
    }
    for (unsigned ichan=0; ichan<nchan; ichan++)
    {
      if (unpacked[isamp][ichan].size() != npol)
      {
        unpacked[isamp][ichan].resize(npol);
      }
    }
  }
}

void ska::pst::common::DataUnpacker::reset()
{
  for (unsigned ichan=0; ichan<nchan; ichan++)
  {
    std::fill(bandpass[ichan].begin(), bandpass[ichan].end(), 0);
  }
  invalid_packets = 0;
  invalid_samples = 0;
}

auto ska::pst::common::DataUnpacker::unpack(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz) -> std::vector<std::vector<std::vector<std::complex<float>>>>&
{
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack data={} data_bufsz={} weights={} weights_bufsz={}",
    reinterpret_cast<void*>(data), data_bufsz, reinterpret_cast<void *>(weights), weights_bufsz);

  resize(data_bufsz);

  const uint32_t nheaps = data_bufsz / layout.get_data_heap_stride();
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack data_heap_stride={} nheaps={}", layout.get_data_heap_stride(), nheaps);

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack packets_per_heap={} npol={} nchan_per_packet={} nsamp_per_packet={}",
    layout.get_packets_per_heap(), npol, layout.get_packet_layout().get_nchan_per_packet(), layout.get_packet_layout().get_samples_per_packet());

  // unpack the 8 or 16 bit signed integers
  if (nbit == 8) // NOLINT
  {
    unpack_samples(reinterpret_cast<int8_t*>(data), weights, nheaps);
  }
  else if (nbit == 16) // NOLINT
  {
    unpack_samples(reinterpret_cast<int16_t*>(data), weights, nheaps);
  }

  if (invalid_packets > 0)
  {
    SPDLOG_WARN("ska::pst::common::DataUnpacker::unpack found {} dropped packets resulting in {} invalid samples", invalid_packets, invalid_samples);
  }

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack unpacking complete");
  return unpacked;
}

void ska::pst::common::DataUnpacker::integrate_bandpass(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack integrate_bandpass={} data_bufsz={} weights={} weights_bufsz={}",
    reinterpret_cast<void*>(data), data_bufsz, reinterpret_cast<void *>(weights), weights_bufsz);

  // unpack the heaped CBF/PSR format into TFP ordered timeseries
  const uint32_t nheaps = data_bufsz / layout.get_data_heap_stride();

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::integrate_bandpass nheaps={} packets_per_heap={} npol={} nchan_per_packet={} nsamp_per_packet={}",
    nheaps, layout.get_packets_per_heap(), npol, layout.get_packet_layout().get_nchan_per_packet(), layout.get_packet_layout().get_samples_per_packet());

  // integerate the 8 or 16 bit signed integers
  if (nbit == 8) // NOLINT
  {
    integrate_samples(reinterpret_cast<int8_t*>(data), weights, nheaps);
  }
  else if (nbit == 16) // NOLINT
  {
    integrate_samples(reinterpret_cast<int16_t*>(data), weights, nheaps);
  }

  if (invalid_packets > 0)
  {
    SPDLOG_WARN("ska::pst::common::DataUnpacker::integrate_bandpass found {} dropped packets resulting in {} invalid samples", invalid_packets, invalid_samples);
  }

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::integrate_bandpass unpacking complete");
}
