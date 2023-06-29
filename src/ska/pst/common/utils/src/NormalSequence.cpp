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
#include "ska/pst/common/utils/NormalSequence.h"

#ifdef DEBUG
#include <stdio.h>
#endif

void ska::pst::common::NormalSequence::configure(const ska::pst::common::AsciiHeader& header)
{
  std::string utc_start_str = header.get_val("UTC_START");
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::configure UTC_START={}", utc_start_str);

  nbit = header.get_uint32("NBIT");
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::configure NBIT={}", nbit);

  min_val = powf(2, nbit-1) * -1;
  max_val = powf(2, nbit-1) - 1;

  if (header.has("DISTRIBUTION_MEAN"))
  {
    mean = header.get_float("DISTRIBUTION_MEAN");
  }
  if (header.has("DISTRIBUTION_STDDEV"))
  {
    stddev = header.get_float("DISTRIBUTION_STDDEV");
  }
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::configure mean={} stddev={}", mean, stddev);

  ska::pst::common::Time utc_start(utc_start_str.c_str());
  seed_value = uint64_t(utc_start.get_time());
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::configure seed_value={}", seed_value);

  reset();
}

void ska::pst::common::NormalSequence::reset()
{
  // reinitialise the internal state of the random-number engine
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::reset generator.seed({})", seed_value);
  generator.seed(seed_value);
}

auto ska::pst::common::NormalSequence::get_val(std::normal_distribution<float>& distribution) -> int16_t
{
  return int16_t(rintf(std::min(std::max(distribution(generator), min_val), max_val)));
}

void ska::pst::common::NormalSequence::generate(char * buffer, uint64_t bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::generate generating {} bytes of normal data", bufsz);
  std::normal_distribution<float> distribution(mean, stddev);

  // floating point values are requantised to signed integers at 8, 12 or 16 bits per sample
  if (nbit == 8)
  {
    for (unsigned i=0; i<bufsz; i++)
    {
      buffer[i] = int8_t(get_val(distribution));
    }
  }
  else if (nbit == 12)
  {
    if (bufsz % 3 != 0)
    {
      spdlog::error("ska::pst::common::NormalSequence::generate bufsz[{}] was not a multiple of 24-bits", bufsz*8);
      throw std::runtime_error("ska::pst::common::NormalSequence::generate bufsz did not align with NBIT");
    }

    for (unsigned i=0; i<bufsz; i+=3)
    {
      // shift from lower 12-bits of int16 to upper 12 bits by 2^4 multiplication
      const int16_t v1 = get_val(distribution) * 16;
      const int16_t v2 = get_val(distribution) * 16;

      // lower 8 bits first value
      buffer[i+0] = char((v1 & 0x0FF0) >> 4);

      // upper 4 bits first, lower 4 bits second value
      buffer[i+1] = char((v1 & 0xF000) >> 12) & char((v2 & 0x00F0) >> 4);

      // upper 8 bits second value
      buffer[i+3] = char((v2 & 0xFF00) >> 8);
    }
  }
  else if (nbit == 16)
  {
    int16_t * to = reinterpret_cast<int16_t*>(buffer);
    uint64_t to_size = bufsz / sizeof(int16_t);
    for (unsigned i=0; i<to_size; i++)
    {
      to[i] = get_val(distribution);
    }
  }

  byte_offset += bufsz;
}

auto ska::pst::common::NormalSequence::validate(char * buffer, uint64_t bufsz) -> bool
{
  // generate a new set a data
  std::vector<char> data(bufsz);
  generate(&data[0], bufsz);

  bool valid = true;
  for (unsigned i=0; i<bufsz; i++)
  {
    if (buffer[i] != data[i])
    {
      valid = false;
    }
  }

  return valid;
}