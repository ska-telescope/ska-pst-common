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

#include "ska/pst/common/utils/SegmentGenerator.h"
#include "ska/pst/common/utils/PacketGeneratorFactory.h"
#include "ska/pst/common/utils/Time.h"

#include "ska/pst/common/definitions.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

ska::pst::common::SegmentGenerator::~SegmentGenerator()
{
  resize (0);
}

void ska::pst::common::SegmentGenerator::configure(const AsciiHeader& _data_config, const AsciiHeader& _weights_config)
{
  data_config = _data_config;
  weights_config = _weights_config;

  update_config(data_config);

  SPDLOG_DEBUG("ska::pst::common::SegmentGenerator::configure calling HeapLayout::initialise");

  layout.initialise(data_config, weights_config);

  std::string generator_name = data_config.get_val("DATA_GENERATOR");
  generator = PacketGeneratorFactory(generator_name, layout.get_packet_layout_ptr());
  generator->configure(data_config);
}

void ska::pst::common::SegmentGenerator::update_config(AsciiHeader& config)
{
  std::string utc_start;

  if (config.has("UTC_START"))
  {
    utc_start = config.get_val("UTC_START");
  }
  else
  {
    ska::pst::common::Time now(time(nullptr));
    utc_start = now.get_gmtime();
    config.set_val("UTC_START",utc_start);
  }

  uint32_t file_number = 0;
  
  if (config.has("FILE_NUMBER"))
  {
    file_number = config.get_uint32("FILE_NUMBER");
  }
  else
  {
    config.set("FILE_NUMBER",file_number);
  }

  uint32_t obs_offset = 0;

  if (config.has("OBS_OFFSET"))
  {
    obs_offset = config.get_uint32("OBS_OFFSET");
  }
  else
  {
    config.set("OBS_OFFSET",obs_offset);
  }
}

void resize(ska::pst::common::BlockProducer::Block& block, uint64_t size)
{
  if (block.block != nullptr && block.size > 0 && (size > block.size || size == 0))
  {
    free (block.block); // NOLINT
    block.block = nullptr;
    block.size = 0;
  }

  if (block.block == nullptr && size > 0)
  {
    static constexpr uint64_t memory_alignment = 512;
    posix_memalign(reinterpret_cast<void **>(&(block.block)), memory_alignment, size);
    block.size = size;
  }
}

void ska::pst::common::SegmentGenerator::resize(uint64_t _nheap)
{
  if (layout.get_data_heap_stride() == 0)
  {
    SPDLOG_ERROR("ska::pst::common::SegmentGenerator::resize data_heap_stride is zero");
    throw std::runtime_error("ska::pst::common::SegmentGenerator::resize data_heap_stride is zero");
  }

  ::resize(segment.data, _nheap * layout.get_data_heap_stride());

  if (layout.get_weights_heap_stride() == 0)
  {
    SPDLOG_ERROR("ska::pst::common::SegmentGenerator::resize weights_heap_stride is zero");
    throw std::runtime_error("ska::pst::common::SegmentGenerator::resize weights_heap_stride is zero");
  }

  ::resize(segment.weights, _nheap * layout.get_weights_heap_stride());

  nheap = _nheap;
}


auto ska::pst::common::SegmentGenerator::next_segment() -> Segment
{
  if (nheap == 0)
  {
    SPDLOG_ERROR("ska::pst::common::SegmentGenerator::next_segment nheap is zero");
    throw std::runtime_error("ska::pst::common::SegmentGenerator::next_segment nheap is zero");
  }

  const uint32_t packets_per_heap = layout.get_packets_per_heap();
  const uint64_t num_packets = nheap * packets_per_heap;

  const PacketLayout& packet_layout = layout.get_packet_layout();

  for (auto packet_number = 0; packet_number < num_packets; packet_number++)
  {
    auto data_offset = packet_number * layout.get_data_packet_stride();
    auto weights_offset = packet_number * layout.get_weights_packet_stride();

    SPDLOG_TRACE("ska::pst::common::SegmentGenerator::next_segment generating data packet {}", packet_number);
    generator->fill_data(segment.data.block + data_offset, packet_layout.get_packet_data_size()); // NOLINT
    SPDLOG_TRACE("ska::pst::common::SegmentGenerator::next_segment generating scales packet {}", packet_number);
    generator->fill_scales(segment.weights.block + weights_offset + packet_layout.get_packet_scales_offset(), packet_layout.get_packet_scales_size()); // NOLINT
    SPDLOG_TRACE("ska::pst::common::SegmentGenerator::next_segment generating weights packet {}", packet_number);
    generator->fill_weights(segment.weights.block + weights_offset + packet_layout.get_packet_weights_offset(), packet_layout.get_packet_weights_size()); // NOLINT
  }

  return segment;
}

auto ska::pst::common::SegmentGenerator::test_segment(const Segment& test) -> bool
{
  if (nheap == 0)
  {
    SPDLOG_ERROR("ska::pst::common::SegmentGenerator::test_segment nheap is zero");
    throw std::runtime_error("ska::pst::common::SegmentGenerator::test_segment nheap is zero");
  }

  const uint32_t packets_per_heap = layout.get_packets_per_heap();
  const uint64_t num_packets = nheap * packets_per_heap;

  const PacketLayout& packet_layout = layout.get_packet_layout();

  for (auto packet_number = 0; packet_number < num_packets; packet_number++)
  {
    auto data_offset = packet_number * layout.get_data_packet_stride();
    auto weights_offset = packet_number * layout.get_weights_packet_stride();

    SPDLOG_TRACE("ska::pst::common::SegmentGenerator::test_segment generating data packet {}", packet_number);
    if (generator->test_data(test.data.block + data_offset, packet_layout.get_packet_data_size()) == false) // NOLINT
    {
      return false;
    }
    SPDLOG_TRACE("ska::pst::common::SegmentGenerator::test_segment generating scales packet {}", packet_number);
    if (generator->test_scales(test.weights.block + weights_offset + packet_layout.get_packet_scales_offset(), packet_layout.get_packet_scales_size()) == false) // NOLINT
    {
      return false;
    }
    SPDLOG_TRACE("ska::pst::common::SegmentGenerator::test_segment generating weights packet {}", packet_number);
    if (generator->test_weights(test.weights.block + weights_offset + packet_layout.get_packet_weights_offset(), packet_layout.get_packet_weights_size()) == false) // NOLINT
    {
      return false;
    }
  }

  return true;
}
      
void ska::pst::common::SegmentGenerator::reset()
{
  generator->reset();
}

