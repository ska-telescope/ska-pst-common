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
class HeapPacketLayout : public ska::pst::common::PacketLayout
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

    if (nsamp_per_packet == 0)
    {
      SPDLOG_ERROR("ska::pst::common::HeapPacketLayout::configure UDP_NSAMP is zero");
      throw std::runtime_error("ska::pst::common::HeapPacketLayout::configure UDP_NSAMP is zero");
    } 

    if (nchan_per_packet == 0)
    {
      SPDLOG_ERROR("ska::pst::common::HeapPacketLayout::configure UDP_NCHAN is zero");
      throw std::runtime_error("ska::pst::common::HeapPacketLayout::configure UDP_NCHAN is zero");
    } 

    if (nsamp_per_weight == 0)
    {
      SPDLOG_ERROR("ska::pst::common::HeapPacketLayout::configure WT_NSAMP is zero");
      throw std::runtime_error("ska::pst::common::HeapPacketLayout::configure WT_NSAMP is zero");
    } 

    if (nsamp_per_packet % nsamp_per_weight != 0)
    {
      SPDLOG_ERROR("ska::pst::common::HeapLayout::configure UDP_NSAMP={} is not a multiple of WT_NSAMP={}", nsamp_per_packet, nsamp_per_weight);
      throw std::runtime_error("ska::pst::common::HeapLayout::configure UDP_NSAMP is not a multiple of WT_NSAMP");
    }
    auto nweight_per_packet = nchan_per_packet * (nsamp_per_packet / nsamp_per_weight);

    // if PACKET_SCALES_SIZE is not set, assume one scale factor per packet
    packet_scales_size = sizeof(float); 
    if (weights_config.has("PACKET_SCALES_SIZE"))
    {
      packet_scales_size = weights_config.get_uint32("PACKET_SCALES_SIZE");
    }

    uint32_t weights_nbit = weights_config.get_uint32("NBIT");
    packet_weights_size = (nweight_per_packet * weights_nbit) / ska::pst::common::bits_per_byte;
    if (weights_config.has("PACKET_WEIGHTS_SIZE"))
    {
      auto config_packet_weights_size = weights_config.get_uint32("PACKET_WEIGHTS_SIZE");
      if (config_packet_weights_size != packet_weights_size)
      {
        SPDLOG_ERROR("ska::pst::common::HeapPacketLayout::configure PACKET_WEIGHTS_SIZE={} in weights_config does not equal packet_weights_size={}", config_packet_weights_size, packet_weights_size);
        throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid PACKET_WEIGHTS_SIZE in weights_config");
      }
    }

    packet_data_size = (nsamp_per_packet * nchan_per_packet * ndim * npol * nbit) / ska::pst::common::bits_per_byte;
    packet_size = packet_data_size;

    // scales are included at the start of each weights block
    packet_scales_offset = 0;
    
    // weights directly follow scales in each weights block
    packet_weights_offset = packet_scales_size;

    // data are included at the start of each data block
    packet_data_offset = 0;

    // no headers in blocks
    packet_header_size = 0;
  }
};

void ska::pst::common::HeapLayout::configure(const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config)
{
  packet_layout = std::make_shared<HeapPacketLayout>(data_config, weights_config);

  auto nchan_per_packet = packet_layout->get_nchan_per_packet();
  auto nsamp_per_packet = packet_layout->get_samples_per_packet();
  
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

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::configure nchan={} nchan_per_packet={}", nchan, nchan_per_packet);
  if (nchan % nchan_per_packet)
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure NCHAN={} was not a multiple of nchan_per_packet={}", nchan, nchan_per_packet);
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid NCHAN is not a multiple of UDP_NCHAN");
  }

  // extract parameters from the weights header
  weights_packet_stride = packet_layout->get_packet_weights_size() + packet_layout->get_packet_scales_size();
  weights_heap_stride = weights_packet_stride * nchan / nchan_per_packet;

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::configure weights packet_stride={}", weights_packet_stride);

  data_packet_stride = (nsamp_per_packet * nchan_per_packet * npol * ndim * nbit) / ska::pst::common::bits_per_byte;
  data_heap_stride = (nsamp_per_packet * nchan * npol * ndim * nbit) / ska::pst::common::bits_per_byte;

  if (data_heap_stride % data_packet_stride)
  {
    SPDLOG_ERROR("ska::pst::common::HeapLayout::configure data_heap_stride={} is not a multiple of data_packet_stride={}", data_heap_stride, data_packet_stride);
    throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid heap/packet stride");
  }
  packets_per_heap = data_heap_stride / data_packet_stride;

  if (data_config.has("RESOLUTION"))
  {
    auto data_resolution = data_config.get_uint32("RESOLUTION");
    if (data_resolution != data_heap_stride)
    {
      SPDLOG_ERROR("ska::pst::common::HeapLayout::configure RESOLUTION={} in data_config does not equal data_heap_stride={}", data_resolution, data_heap_stride);
      throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid RESOLUTION in data_config");
    }
  }

  if (weights_config.has("RESOLUTION"))
  {
    auto weights_resolution = weights_config.get_uint32("RESOLUTION");
    if (weights_resolution != weights_heap_stride)
    {
      SPDLOG_ERROR("ska::pst::common::HeapLayout::configure RESOLUTION={} in weights_config does not equal weights_heap_stride={}", weights_resolution, weights_heap_stride);
      throw std::runtime_error("ska::pst::common::HeapLayout::configure invalid RESOLUTION in weights_config");
    }
  }

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::configure exit");
}

void ska::pst::common::HeapLayout::initialise(ska::pst::common::AsciiHeader& data_config, ska::pst::common::AsciiHeader& weights_config)
{
  SPDLOG_DEBUG("ska::pst::common::HeapLayout::initialise calling configure");

  configure(data_config, weights_config);

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::initialise data resolution={}", data_heap_stride);
  data_config.set("RESOLUTION",data_heap_stride);

  SPDLOG_DEBUG("ska::pst::common::HeapLayout::initialise weights resolution={}", weights_heap_stride);
  weights_config.set("RESOLUTION",weights_heap_stride);

  weights_config.set("PACKET_WEIGHTS_SIZE",packet_layout->get_packet_weights_size());
  weights_config.set("PACKET_SCALES_SIZE",packet_layout->get_packet_scales_size());
}
