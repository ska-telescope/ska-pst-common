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

  if (header.has("NORMAL_DIST_MEAN"))
  {
    mean = header.get_float("NORMAL_DIST_MEAN");
  }
  if (header.has("NORMAL_DIST_STDDEV"))
  {
    stddev = header.get_float("NORMAL_DIST_STDDEV");
  }
  if (header.has("NORMAL_DIST_RED_STDDEV"))
  {
    red_stddev = header.get_float("NORMAL_DIST_RED_STDDEV");
  }

  SPDLOG_DEBUG("ska::pst::common::NormalSequence::configure mean={} stddev={} red_stddev={}", mean, stddev, red_stddev);

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
  red_noise_generator.seed(seed_value+1);
}

auto ska::pst::common::NormalSequence::get_val(std::normal_distribution<float>& distribution) -> int16_t
{
  float value = distribution(generator);

  // apply an optional red noise process
  if (red_stddev > 0)
  {
    red_noise_factor = (0.9 * red_noise_factor) + (0.1 * new_red_noise_factor);
    value *= red_noise_factor;
  }

  return int16_t(rintf(std::min(std::max(value, min_val), max_val)));
}

void ska::pst::common::NormalSequence::generate(char * buffer, uint64_t bufsz)
{
  SPDLOG_DEBUG("ska::pst::common::NormalSequence::generate generating {} bytes of normal data", bufsz);

  if (red_stddev > 0)
  {
    // update the new red noise factor once per buffer
    std::normal_distribution<float> red_noise_distribution(0, red_stddev);
    new_red_noise_factor = red_noise_distribution(red_noise_generator);
  }

  uint64_t nval = bufsz * ska::pst::common::bits_per_byte / nbit;
  // floating point values are requantised to signed integers at 8 or 16 bits per sample
  if (nbit == 8)
  {
    generate_samples(reinterpret_cast<int8_t *>(buffer), nval);
  }
  else if (nbit == 16)
  {
    generate_samples(reinterpret_cast<int16_t *>(buffer), nval);
  }
  byte_offset += bufsz;
}

auto ska::pst::common::NormalSequence::validate(char * buffer, uint64_t bufsz) -> bool
{
  // generate a new set a data
  std::vector<char> data(bufsz);
  generate(&data[0], bufsz);

  for (unsigned i=0; i<bufsz; i++)
  {
    if (buffer[i] != data[i])
    {
      return false;
    }
  }

  return true;
}
