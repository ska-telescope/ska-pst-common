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

#include <spdlog/spdlog.h>

#include <utility>

#include "ska/pst/common/utils/ScaleWeightGenerator.h"

ska::pst::common::ScaleWeightGenerator::ScaleWeightGenerator(std::shared_ptr<ska::pst::common::PacketLayout> _layout) :
  PacketGenerator(std::move(_layout)), wts_sequence(unity_weight), scl_sequence(unity_scale)
{
}

void ska::pst::common::ScaleWeightGenerator::configure(const ska::pst::common::AsciiHeader& config)
{
  SPDLOG_DEBUG("ska::pst::common::ScaleWeightGenerator::configure");
  ska::pst::common::PacketGenerator::configure(config);
  wts_sequence.configure(config);
  scl_sequence.configure(config);
}

void ska::pst::common::ScaleWeightGenerator::fill_weights(char * buf, uint64_t size)
{
  SPDLOG_TRACE("ska::pst::common::ScaleWeightGenerator::fill_weights buf={} size={}", reinterpret_cast<void*>(buf), size);
  wts_sequence.generate_block(buf, size, wts_block_offset, wts_block_size, block_stride);
}

void ska::pst::common::ScaleWeightGenerator::fill_scales(char * buf, uint64_t size)
{
  SPDLOG_TRACE("ska::pst::common::ScaleWeightGenerator::fill_scales buf={} size={}", reinterpret_cast<void*>(buf), size);
  scl_sequence.generate_block(buf, size, scl_block_offset, scl_block_size, block_stride);
}

auto ska::pst::common::ScaleWeightGenerator::test_weights(char * buf, uint64_t size) -> bool
{
  SPDLOG_TRACE("ska::pst::common::ScaleWeightGenerator::test_weights buf={} size={}", reinterpret_cast<void*>(buf), size);
  return wts_sequence.validate_block(buf, size, wts_block_offset, wts_block_size, block_stride);
}

auto ska::pst::common::ScaleWeightGenerator::test_scales(char * buf, uint64_t size) -> bool
{
  SPDLOG_TRACE("ska::pst::common::ScaleWeightGenerator::test_scales buf={} size={}", reinterpret_cast<void*>(buf), size);
  return scl_sequence.validate_block(buf, size, scl_block_offset, scl_block_size, block_stride);
}

void ska::pst::common::ScaleWeightGenerator::reset()
{
  wts_sequence.reset();
  scl_sequence.reset();
}
