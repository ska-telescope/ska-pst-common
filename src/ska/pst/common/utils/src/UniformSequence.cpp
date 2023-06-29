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
#include <climits>

#include "ska/pst/common/utils/Time.h"
#include "ska/pst/common/utils/UniformSequence.h"

#ifdef DEBUG
#include <stdio.h>
#endif

ska::pst::common::UniformSequence::UniformSequence(const char value) :
  uniform_value(value)
{
  SPDLOG_DEBUG("ska::pst::common::UniformSequence::UniformSequence uniform_value={}", uniform_value);
}

void ska::pst::common::UniformSequence::configure(const ska::pst::common::AsciiHeader& header)
{
  reset();
}

void ska::pst::common::UniformSequence::reset()
{
}

void ska::pst::common::UniformSequence::generate(char * buffer, uint64_t bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::UniformSequence::generate generate {} bytes of normal data", bufsz);
  std::fill(buffer, buffer + bufsz, uniform_value);
}

void ska::pst::common::UniformSequence::generate_block(char * buffer, uint64_t bufsz, uint64_t block_offset, uint64_t block_size, uint64_t block_stride)
{
  SPDLOG_DEBUG("ska::pst::common::UniformSequence::generate_block generate {} bytes of normal data with block offset={}, size={} and stride={}", bufsz, block_offset, block_size, block_stride);
  uint64_t offset = block_offset;
  while (offset + block_size < bufsz)
  {
    generate(buffer + offset, offset + block_size);
    offset += block_stride;
  }
}

auto ska::pst::common::UniformSequence::validate(char * buffer, uint64_t bufsz) -> bool
{
  for (unsigned i=0; i<bufsz; i++)
  {
    if (buffer[i] != uniform_value)
    {
      return false;
    }
  }
  return true;
}

auto ska::pst::common::UniformSequence::validate_block(char * buffer, uint64_t bufsz, uint64_t block_offset, uint64_t block_size, uint64_t block_stride) -> bool
{
  SPDLOG_DEBUG("ska::pst::common::UniformSequence::validate_block validate {} bytes of normal data with block offset={}, size={} stride={}", bufsz, block_offset, block_size, block_stride);
  uint64_t offset = block_offset;
  bool valid = true;
  while (offset + block_size < bufsz)
  {
    valid &= validate(buffer + offset, offset + block_size);
    offset += block_stride;
  }
  SPDLOG_DEBUG("ska::pst::common::UniformSequence::validate_block valid={}", valid);
  return valid;
}
