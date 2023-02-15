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

  // determine the packet weights size and packet scales size
  // packet_weights_size = (nsamp_per_packet / nsamp_per_weight) * nchan_per_packet * nbit) / ska::pst::common::bits_per_byte;
  // packet_scales_size = sizeof(float);
  // weights_packet_stride = packet_weights_size + packet_weights_size;

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
  if (isnanf(weights_ptr[0]))
  {
    SPDLOG_WARN("ska::pst::common::DataUnpacker::get_scale_factor scale factor for packet {} was NaN", packet_number);
  }
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
}

std::vector<std::vector<std::vector<std::complex<float>>>>& ska::pst::common::DataUnpacker::unpack(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack data={} data_bufsz={} weights={} weights_bufsz={}",
    reinterpret_cast<void*>(data), data_bufsz, reinterpret_cast<void *>(weights), weights_bufsz);

  // packet counter
  uint32_t packet_number = 0;

  // pointers for reading different bit-widths
  int8_t * input8 = reinterpret_cast<int8_t *>(data);
  int16_t * input16 = reinterpret_cast<int16_t *>(data);

  resize(data_bufsz);

  // unpack the heaped CBF/PSR format into TFP ordered timeseries
  const uint32_t osamp_stride = nchan * npol;
  const uint32_t nheaps = data_bufsz / heap_resolution;
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack data_bufsz={} heap_resolution={} nheaps={}", data_bufsz, heap_resolution, nheaps);
  uint64_t i = 0;

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack nheaps={} packets_per_heap={} npol={} nchan_per_packet={} nsamp_per_packet={}",
    nheaps, packets_per_heap, npol, nchan_per_packet, nsamp_per_packet);

  for (uint32_t iheap=0; iheap<nheaps; iheap++)
  {
    const uint32_t heap_osamp = iheap * nsamp_per_packet;
    for (uint32_t ipacket=0; ipacket<packets_per_heap; ipacket++)
    {
      const uint32_t packet_ochan = ipacket * nchan_per_packet;

      // read the scale factor from the weights stream
      const float scale_factor = get_scale_factor(weights, packet_number);

      for (uint32_t ipol=0; ipol<npol; ipol++)
      {
        for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
        {
          const uint32_t ochan = packet_ochan + ichan;

          // unpack 2 samples per iteration
          for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp+=2)
          {
            uint32_t osamp = heap_osamp + isamp;

            float real_s0, imag_s0, real_s1, imag_s1;
            if (nbit == 8)
            {
              real_s0 = float(input8[i]);
              imag_s0 = float(input8[i+1]);
              real_s1 = float(input8[i+2]);
              imag_s1 = float(input8[i+3]);
              i += 4;
            }
            else if (nbit == 12)
            {
              real_s0 = float(input16[i] >> 4);
              imag_s0 = float(((input16[i] & 0x000F) << 8) & (input16[i+1] >> 8));
              real_s1 = float(((input16[i+1] & 0x00FF) << 4) & (input16[i+2] >> 12));
              imag_s1 = float(input16[i+2] & 0x0FFF);
              i += 3;
            }
            else if (nbit == 16)
            {
              real_s0 = float(input16[i]);
              imag_s0 = float(input16[i+1]);
              real_s1 = float(input16[i+2]);
              imag_s1 = float(input16[i+3]);
              i += 4;
            }
            unpacked[osamp][ochan][ipol] = std::complex<float>(real_s0 / scale_factor, imag_s0 / scale_factor);
            unpacked[osamp+1][ochan][ipol] = std::complex<float>(real_s1 / scale_factor, imag_s1 / scale_factor);
          }
        }
      }
      packet_number++;
    }
  }

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack unpacking complete");
  return unpacked;
}

void ska::pst::common::DataUnpacker::integrate_bandpass(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::unpack integrate_bandpass={} data_bufsz={} weights={} weights_bufsz={}",
    reinterpret_cast<void*>(data), data_bufsz, reinterpret_cast<void *>(weights), weights_bufsz);

  // packet counter
  uint32_t packet_number = 0;

  // pointers for reading different bit-widths
  int8_t * input8 = reinterpret_cast<int8_t *>(data);
  int16_t * input16 = reinterpret_cast<int16_t *>(data);

  // unpack the heaped CBF/PSR format into TFP ordered timeseries
  const uint32_t osamp_stride = nchan * npol;
  const uint32_t nheaps = data_bufsz / heap_resolution;
  uint64_t i = 0;

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::integrate_bandpass nheaps={} packets_per_heap={} npol={} nchan_per_packet={} nsamp_per_packet={}",
    nheaps, packets_per_heap, npol, nchan_per_packet, nsamp_per_packet);

  for (uint32_t iheap=0; iheap<nheaps; iheap++)
  {
    const uint32_t heap_osamp = iheap * nsamp_per_packet;
    for (uint32_t ipacket=0; ipacket<packets_per_heap; ipacket++)
    {
      const uint32_t packet_ochan = ipacket * nchan_per_packet;

      // read the scale factor from the weights stream
      const float scale_factor = get_scale_factor(weights, packet_number);

      for (uint32_t ipol=0; ipol<npol; ipol++)
      {
        for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
        {
          const uint32_t ochan = packet_ochan + ichan;

          // unpack 2 samples per iteration
          for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp+=2)
          {
            uint32_t osamp = heap_osamp + isamp;

            float real_s0, imag_s0, real_s1, imag_s1;
            if (nbit == 8)
            {
              real_s0 = float(input8[i]) / scale_factor;
              imag_s0 = float(input8[i+1]) / scale_factor;
              real_s1 = float(input8[i+2]) / scale_factor;
              imag_s1 = float(input8[i+3]) / scale_factor;
              i += 4;
            }
            else if (nbit == 12)
            {
              real_s0 = float(input16[i] >> 4) / scale_factor;
              imag_s0 = float(((input16[i] & 0x000F) << 8) & (input16[i+1] >> 8)) / scale_factor;
              real_s1 = float(((input16[i+1] & 0x00FF) << 4) & (input16[i+2] >> 12)) / scale_factor;
              imag_s1 = float(input16[i+2] & 0x0FFF) / scale_factor;
              i += 3;
            }
            else if (nbit == 16)
            {
              real_s0 = float(input16[i]) / scale_factor;
              imag_s0 = float(input16[i+1]) / scale_factor;
              real_s1 = float(input16[i+2]) / scale_factor;
              imag_s1 = float(input16[i+3]) / scale_factor;
              i += 4;
            }

            bandpass[ochan][ipol] += (real_s0 * real_s0) + (imag_s0 * imag_s0) + (real_s1 * real_s1) + (imag_s1 * imag_s1);
          }
        }
      }
      packet_number++;
    }
  }

  SPDLOG_DEBUG("ska::pst::common::DataUnpacker::integrate_bandpass unpacking complete");
}
