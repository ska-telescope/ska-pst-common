/*
 * Copyright 2022 Square Kilometre Array Observatory
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
#include "ska/pst/common/utils/RandomSequence.h"

#ifdef DEBUG
#include <stdio.h>
#endif

void ska::pst::common::RandomSequence::configure(const ska::pst::common::AsciiHeader& header)
{
  // to enable debug-level verbosity (when make build_debug doesn't)
  // spdlog::set_level(spdlog::level::debug);

  std::string utc_start_str = header.get_val("UTC_START");
  SPDLOG_DEBUG("ska::pst::common::RandomSequence::configure UTC_START={}", utc_start_str);

  ska::pst::common::Time utc_start(utc_start_str.c_str());
  seed_value = uint64_t(utc_start.get_time());
  SPDLOG_DEBUG("ska::pst::common::RandomSequence::configure seed_value={}", seed_value);

  reset();
  uint64_t obs_offset = header.get_uint64("OBS_OFFSET");
  SPDLOG_DEBUG("ska::pst::common::RandomSequence::configure OBS_OFFSET={}", obs_offset);
  seek(obs_offset);
}

void ska::pst::common::RandomSequence::reset()
{
  // reinitialise the internal state of the random-number engine
  SPDLOG_DEBUG("ska::pst::common::RandomSequence::reset generator.seed({})", seed_value);
  generator.seed(seed_value);
}

void ska::pst::common::RandomSequence::generate(uint8_t * buffer, uint64_t bufsz)
{
  // produces random integer values, uniformly distributed on the closed interval
  std::uniform_int_distribution<uint8_t> distribution(0, UCHAR_MAX);
  for (unsigned i=0; i<bufsz; i++)
  {
    buffer[i] = distribution(generator); // NOLINT
  }

  byte_offset += bufsz;
}

void ska::pst::common::RandomSequence::seek(uint64_t bufsz)
{
  // advances the internal state of the generator bufsz steps
  // note that the generator's discard method is unreliable, hence this manual step
  std::uniform_int_distribution<uint8_t> distribution(0, UCHAR_MAX);
  for (uint64_t i=0; i<bufsz; i++)
  {
    distribution(generator);
  }
}

auto ska::pst::common::RandomSequence::validate(uint8_t * buffer, uint64_t bufsz) -> bool
{
  // produces random integer values, uniformly distributed on the closed interval
  std::uniform_int_distribution<uint8_t> distribution(0, UCHAR_MAX);

  uint64_t i = 0; 
  while (i<bufsz)
  {
    const uint8_t expected = distribution(generator);

    if (buffer[i] == expected)  // NOLINT
    {
      i ++;
    }
    else
    {
      spdlog::warn("ska::pst::common::RandomSequence::validate unexpected byte at index={}", i);

      unsigned zeroes = 0;
      while (i<bufsz && buffer[i] == 0) // NOLINT
      {
        i++;
        zeroes++;
      }

      if (zeroes > 1)
      {
        spdlog::warn("ska::pst::common::RandomSequence::validate skipping {} consecutive zeroes", zeroes);
        seek (zeroes - 1);
      }
      else
      {
        // sequence is broken (data invalid)
        SPDLOG_ERROR("ska::pst::common::RandomSequence::validate expected sequence broken at i={}; bufsz={}", i, bufsz);
        break;
      }
    }
  }

  byte_offset += i;

  if (i == bufsz)
  {
    // sequence was not broken (data valid)
    return true;
  }
 
  uint64_t start_search_index = i + 1;
  uint8_t* remaining_buffer = buffer + start_search_index; // NOLINT
  uint64_t remaining_bufsz = bufsz - start_search_index;

  const unsigned expected_sequence_length = 8;
  if (search_buffer_for_expected_sequence (remaining_buffer, remaining_bufsz, expected_sequence_length) == -1)
  {
    start_search_index += expected_sequence_length;
    if (start_search_index < bufsz)
    {
      remaining_buffer = buffer + start_search_index; // NOLINT
      remaining_bufsz = bufsz - start_search_index;

      const unsigned max_offset = 8 * 1024 * 1024;
      search_expected_sequence_for_buffer (remaining_buffer, remaining_bufsz, max_offset);
    }
  }

  // sequence was broken (data invalid)
  return false;
}

auto ska::pst::common::RandomSequence::search_buffer_for_expected_sequence (uint8_t * buffer, uint64_t bufsz, uint64_t seqlen) -> int64_t
{
  // avoid searching for a sequence longer than the input buffer
  seqlen = std::min (seqlen, bufsz);

  std::uniform_int_distribution<uint8_t> distribution(0, UCHAR_MAX);
  std::vector<uint8_t> test_sequence (seqlen);

  std::string sequence_str;
  for (uint64_t i=1; i<seqlen; i++)
  {
    test_sequence[i] = distribution(generator);

    const unsigned tmpsize = 8;
    std::array<char,tmpsize> temp;
    snprintf (temp.data(), tmpsize, " %02x", test_sequence[i]);
    sequence_str += temp.data();
  }

  spdlog::warn("ska::pst::common::RandomSequence::search_buffer_for_expected_sequence: {}", sequence_str);

  unsigned matched = 0;
  uint64_t i = 0;

  while (i < bufsz && matched < seqlen)
  {
    if (buffer[i] == test_sequence[matched]) // NOLINT
    {
      matched ++;
    }
    else
    {
      matched = 0;
    }
    i++;
  }

  if (matched < seqlen)
  {
    spdlog::warn("ska::pst::common::RandomSequence::search_buffer_for_expected_sequence not found");
    return -1;
  }
      
  spdlog::warn("ska::pst::common::RandomSequence::search_buffer_for_expected_sequence found at offset={}", i-seqlen);
  return i - seqlen;
}

auto ska::pst::common::RandomSequence::search_expected_sequence_for_buffer (uint8_t * buffer, uint64_t bufsz, uint64_t max_offset) -> int64_t
{
  spdlog::warn("ska::pst::common::RandomSequence::search_expected_sequence_for_buffer max offset = {}", max_offset);

  uint64_t matched = 0;
  uint64_t longest_match = 0;
  uint64_t offset = 0;

  std::uniform_int_distribution<uint8_t> distribution(0, UCHAR_MAX);

  while (matched < bufsz && offset < max_offset)
  {
    uint8_t expected = distribution(generator);

    offset ++;

    if (buffer[matched] == expected) // NOLINT
    {
      matched ++;
    }
    else
    {
      longest_match = std::max(matched,longest_match);
      matched = 0;
    }
  }

  if (matched == bufsz)
  {
    spdlog::warn("ska::pst::common::RandomSequence::search_expected_sequence_for_buffer match found at offset={}", offset-bufsz);
    return offset - bufsz;
  }

  spdlog::warn("ska::pst::common::RandomSequence::search_expected_sequence_for_buffer match not found in first {} samples of expected sequence", max_offset);
  spdlog::warn("ska::pst::common::RandomSequence::search_expected_sequence_for_buffer longest match={} out of {} buffer samples", longest_match, bufsz);

  return -1;
}
  

