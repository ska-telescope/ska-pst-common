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
  // extract the required parameters from the data header
  ndim = data_config.get_uint32("NDIM");
  npol = data_config.get_uint32("NPOL");
  nbit = data_config.get_uint32("NBIT");
  nchan = data_config.get_uint32("NCHAN");
  nsamp_per_packet = data_config.get_uint32("UDP_NSAMP");
  nchan_per_packet = data_config.get_uint32("UDP_NCHAN");
  nsamp_per_weight = data_config.get_uint32("WT_NSAMP");

  // extract parameters from the weights header
  weights_nbit = weights_config.get_uint32("NBIT");
  weights_packet_stride = weights_config.get_uint32("PACKET_WEIGHTS_SIZE") + weights_config.get_uint32("PACKET_SCALES_SIZE");
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::configure weights nbit={} packet_stride={}", weights_nbit, weights_packet_stride);

  if (ndim != 2)
  {
    SPDLOG_ERROR("ska::pst::common::DataUnpacker::configure expected NDIM=2, but found {}", ndim);
    throw std::runtime_error("ska::pst::common::DataUnpacker::configure invalid NDIM");
  }

  if (npol != 2)
  {
    SPDLOG_ERROR("ska::pst::common::DataUnpacker::configure expected NPOL=2, but found {}", npol);
    throw std::runtime_error("ska::pst::common::DataUnpacker::configure invalid NPOL");
  }

  if (nbit != 8 && nbit != 12 && nbit != 16)
  {
    SPDLOG_ERROR("ska::pst::common::DataUnpacker::configure expected NBIT=8, 12 or 16, but found {}", nbit);
    throw std::runtime_error("ska::pst::common::DataUnpacker::configure invalid NBIT");
  }

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::configure nchan={} layout.get_nchan_per_packet()={}", nchan, nchan_per_packet);
  if (nchan % nchan_per_packet != 0)
  {
    SPDLOG_ERROR("ska::pst::common::DataUnpacker::configure NCHAN={} was not a multiple of nchan_per_packet={}", nchan, nchan_per_packet);
    throw std::runtime_error("ska::pst::common::SineWaveGenerator::configure invalid NCHAN");
  }

  nsamp_per_weight = nsamp_per_packet; // compliant with all known ICD formats, but not robust to new formats
  packet_resolution = nsamp_per_packet * nchan_per_packet * npol * ndim * nbit / ska::pst::common::bits_per_byte;
  heap_resolution = nsamp_per_packet * nchan * npol * ndim * nbit / ska::pst::common::bits_per_byte;
  if (heap_resolution % packet_resolution)
  {
    SPDLOG_WARN("ska::pst::common::DataUnpacker::configure heap_resolution[{}] was not a multiple of packet resolution[{}]", heap_resolution, packet_resolution);
    throw std::runtime_error("ska::pst::common::DataUnpacker::configure invalid heap/packet resolution");
  }
  packets_per_heap = heap_resolution / packet_resolution;

  bandpass.resize(nchan);
  for (unsigned ichan=0; ichan<nchan; ichan++)
  {
    bandpass[ichan].resize(npol);
  }
  reset();
}

float ska::pst::common::DataUnpacker::get_scale_factor(char * weights, uint32_t packet_number)
{
  SPDLOG_TRACE("ska::pst::common::DataUnpacker::get_scale_factor weights={}, packet_number={} weights_packet_stride={}",
    reinterpret_cast<void *>(weights), packet_number, weights_packet_stride);
  float * weights_ptr = reinterpret_cast<float *>(weights + (packet_number * weights_packet_stride));
  return weights_ptr[0];
}

void ska::pst::common::DataUnpacker::resize(uint64_t data_bufsz)
{
  // determine the unpacked data size in TFP order
  const uint64_t nsamp = (data_bufsz * ska::pst::common::bits_per_byte) / (nchan * npol * ndim * nbit);
  const uint64_t nval = nsamp * nchan * npol;

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::resize nsamp={} nchan={} npol={} nval={}", nsamp, nchan, npol, nval);
  unpacked.resize(nsamp);
  for (unsigned isamp=0; isamp<nsamp; isamp++)
  {
    unpacked[isamp].resize(nchan);
    for (unsigned ichan=0; ichan<nchan; ichan++)
    {
      unpacked[isamp][ichan].resize(npol);
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

std::vector<std::vector<std::vector<std::complex<float>>>>& ska::pst::common::DataUnpacker::unpack(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack data={} data_bufsz={} weights={} weights_bufsz={}",
    reinterpret_cast<void*>(data), data_bufsz, reinterpret_cast<void *>(weights), weights_bufsz);

  resize(data_bufsz);

  const uint32_t nheaps = data_bufsz / heap_resolution;
  const uint32_t input_step = (nbit * 4) / ska::pst::common::bits_per_byte;
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack heap_resolution={} nheaps={}", heap_resolution, nheaps);
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack packets_per_heap={} npol={} nchan_per_packet={} nsamp_per_packet={}",
    packets_per_heap, npol, nchan_per_packet, nsamp_per_packet);

  // Unpack quantised data store in heap, packet, pol, chan_block, samp_block ordering used in CBF/PSR formats
  uint32_t packet_number = 0;
  for (uint32_t iheap=0; iheap<nheaps; iheap++)
  {
    for (uint32_t ipacket=0; ipacket<packets_per_heap; ipacket++)
    {
      const float scale_factor = get_scale_factor(weights, packet_number);
      if (std::isnan(scale_factor))
      {
        invalid_packets++;
      }

      for (uint32_t ipol=0; ipol<npol; ipol++)
      {
        for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
        {
          for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp+=2)
          {
            // unpack a pair of complex samples, normalising by the scale factor
            sample_pair_t sample_pair = unpack_sample_pair(data, scale_factor);

            // compute the output sample and channel
            uint32_t osamp = (iheap * nsamp_per_packet) + isamp;
            uint32_t ochan = (ipacket * nchan_per_packet) + ichan;
            unpacked[osamp+0][ochan][ipol] = sample_pair.sample1;
            unpacked[osamp+1][ochan][ipol] = sample_pair.sample2;

            data += input_step;
          }
        }
      }
      packet_number++;
    }
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
  const uint32_t nheaps = data_bufsz / heap_resolution;
  const uint32_t input_step = (nbit * 4) / ska::pst::common::bits_per_byte;

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::integrate_bandpass nheaps={} packets_per_heap={} npol={} nchan_per_packet={} nsamp_per_packet={}",
    nheaps, packets_per_heap, npol, nchan_per_packet, nsamp_per_packet);

  // Unpack quantised data store in heap, packet, pol, chan_block, samp_block ordering used in CBF/PSR formats
  uint32_t packet_number = 0;
  for (uint32_t iheap=0; iheap<nheaps; iheap++)
  {
    for (uint32_t ipacket=0; ipacket<packets_per_heap; ipacket++)
    {
      const float scale_factor = get_scale_factor(weights, packet_number);
      if (std::isnan(scale_factor))
      {
        invalid_packets++;
      }

      for (uint32_t ipol=0; ipol<npol; ipol++)
      {
        for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
        {
          for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp+=2)
          {
            // unpack a pair of complex samples, normalising by the scale factor
            sample_pair_t sample_pair = unpack_sample_pair(data, scale_factor);
            const uint32_t ochan = (ipacket * nchan_per_packet) + ichan;

            // integrate the power in each complex sample into the bandpass
            bandpass[ochan][ipol] += square_law_detection(sample_pair);
            data += input_step;
          }
        }
      }
      packet_number++;
    }
  }

  if (invalid_packets > 0)
  {
    SPDLOG_WARN("ska::pst::common::DataUnpacker::integrate_bandpass found {} dropped packets resulting in {} invalid samples", invalid_packets, invalid_samples);
  }

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::integrate_bandpass unpacking complete");
}

ska::pst::common::sample_pair_t ska::pst::common::DataUnpacker::unpack_sample_pair(char * input, const float scale_factor)
{
  sample_pair_t unpacked;

  // A NAN scale factor can occur if the drop packets were encountered
  if (std::isnan(scale_factor))
  {
    invalid_samples++;
    unpacked.sample1 = std::complex<float>(0, 0);
    unpacked.sample2 = std::complex<float>(0, 0);
    return unpacked;
  }

  if (nbit == 8)
  {
    int8_t * in = reinterpret_cast<int8_t *>(input);
    unpacked.sample1 = std::complex<float>(float(in[0]), float(in[1]));
    unpacked.sample2 = std::complex<float>(float(in[2]), float(in[3]));
  }
  else if (nbit == 12)
  {
    int16_t * in = reinterpret_cast<int16_t *>(input);
    unpacked.sample1 = std::complex<float>(float(in[0] >> 4), float(((in[0] & 0x000F) << 8) & (in[1] >> 8)));
    unpacked.sample2 = std::complex<float>(float(((in[1] & 0x00FF) << 4) & (in[2] >> 12)), float(in[2] & 0x0FFF));
  }
  else if (nbit == 16)
  {
    int16_t * in = reinterpret_cast<int16_t *>(input);
    unpacked.sample1 = std::complex<float>(float(in[0]), float(in[1]));
    unpacked.sample2 = std::complex<float>(float(in[2]), float(in[3]));
  }

  if (scale_factor != 0)
  {
    unpacked.sample1 /= scale_factor;
    unpacked.sample2 /= scale_factor;
  }
  return unpacked;
}

float ska::pst::common::DataUnpacker::square_law_detection(sample_pair_t unpacked)
{
  return unpacked.sample1.real() * unpacked.sample1.real() +
    unpacked.sample1.imag() * unpacked.sample1.imag() +
    unpacked.sample2.real() * unpacked.sample2.real() +
    unpacked.sample2.imag() * unpacked.sample2.imag();
}
