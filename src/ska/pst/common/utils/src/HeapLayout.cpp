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

#include "ska/pst/common/utils/HeapLayout.h"
#include "ska/pst/common/definitions.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

/**
 * @brief Stores the offsets and sizes of data, weights, and scales of each packet in a heap
 *
 * Data and weights are stored in separate heaps in blocks of shared memory or data from file
 */
class HeapPacketLayout : public ska::pst::common::DataLayout
{
  public:
  HeapPacketLayout(const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config)
  {
    uint32_t ndim = data_config.get_uint32("NDIM");
    uint32_t npol = data_config.get_uint32("NPOL");
    uint32_t nbit = data_config.get_uint32("NBIT");

    nsamp_per_packet = data_config.get_uint32("UDP_NSAMP");
    nchan_per_packet = data_config.get_uint32("UDP_NCHAN");
    nsamp_per_weight = data_config.get_uint32("WT_NSAMP");

    if (nsamp_per_packet % nsamp_per_weight != 0)
    {
      SPDLOG_ERROR("ska::pst::common::HeapLayout::configure UDP_NSAMP={} is not a multiple of WT_NSAMP={}", nsamp_per_packet, nsamp_per_weight);
      throw std::runtime_error("ska::pst::common::HeapLayout::configure UDP_NSAMP is not a multiple of WT_NSAMP");
    }

    // scales are included at the start of each weights block
    packet_scales_size = weights_config.get_uint32("PACKET_SCALES_SIZE"); // NOLINT
    packet_scales_offset = 0;

    // weights directly follow scales in each weights block
    packet_weights_size = weights_config.get_uint32("PACKET_WEIGHTS_SIZE");
    packet_weights_offset = packet_scales_size;

    // data are included at the start of each weights block
    packet_data_size = (nsamp_per_packet * nchan_per_packet * ndim * npol * nbit) / ska::pst::common::bits_per_byte;
    packet_data_offset = 0;

    packet_size = packet_data_size;
    packet_header_size = 0;
  }
};

void ska::pst::common::HeapLayout::configure(const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config)
{
  packet_layout = HeapPacketLayout(data_config, weights_config);

  // extract the required parameters from the data header
  auto ndim = data_config.get_uint32("NDIM");
  auto npol = data_config.get_uint32("NPOL");
  auto nbit = data_config.get_uint32("NBIT");
  auto nchan = data_config.get_uint32("NCHAN");

  if (ndim != 2)
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure expected NDIM=2, but found {}", ndim);
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid NDIM");
  }

  if (npol != 2)
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure expected NPOL=2, but found {}", npol);
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid NPOL");
  }

  if (nbit != 8 && nbit != 16) // NOLINT
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure expected NBIT=8 or 16, but found {}", nbit);
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid NBIT");
  }

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::configure nchan={} nchan_per_packet={}", nchan, packet_layout.get_nchan_per_packet());
  if (nchan % packet_layout.get_nchan_per_packet() != 0)
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure NCHAN={} was not a multiple of nchan_per_packet={}", nchan, packet_layout.get_nchan_per_packet());
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid NCHAN");
  }

  // extract parameters from the weights header
  weights_packet_stride = weights_config.get_uint32("PACKET_WEIGHTS_SIZE") + weights_config.get_uint32("PACKET_SCALES_SIZE");
  weights_heap_stride = weights_packet_stride * nchan / packet_layout.get_nchan_per_packet();

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::configure weights packet_stride={}", weights_packet_stride);

  data_packet_stride = (packet_layout.get_samples_per_packet() * packet_layout.get_nchan_per_packet() * npol * ndim * nbit) / ska::pst::common::bits_per_byte;
  data_heap_stride = (packet_layout.get_samples_per_packet() * nchan * npol * ndim * nbit) / ska::pst::common::bits_per_byte;
  if (data_heap_stride % data_packet_stride)
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure data_heap_stride={} is not a multiple of data_packet_stride={}", data_heap_stride, data_packet_stride);
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid heap/packet stride");
  }
  packets_per_heap = data_heap_stride / data_packet_stride;
}
