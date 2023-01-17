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

#include "ska/pst/common/utils/SineWaveGenerator.h"
#include <spdlog/spdlog.h>
#include <algorithm>

void ska::pst::common::SineWaveGenerator::configure(const ska::pst::common::AsciiHeader& config)
{
  spdlog::debug("ska::pst::common::SineWaveGenerator::configure");

  // might parse the period / frequency of the pure tone from the AsciiHeader ?
}

char ska::pst::common::SineWaveGenerator::next_sample ()
{
  double phase = current_sample / period;
  current_sample ++;

  const double amplitude = 127.0; // output signed char will span -127 to 127
  return static_cast<char> ( amplitude * sin(phase ));
}

void ska::pst::common::SineWaveGenerator::fill_data(char * buf, uint64_t size)
{
  for (uint64_t i=0; i<size; i++)
  {
    buf[i] = next_sample();
  }
}

void ska::pst::common::SineWaveGenerator::fill_weights(char * buf, uint64_t size)
{
  std::fill (buf, buf+size, 0xff);
}

void ska::pst::common::SineWaveGenerator::fill_scales(char * buf, uint64_t size)
{
  std::fill (buf, buf+size, 0xff);
}

auto ska::pst::common::SineWaveGenerator::test_data(char * buf, uint64_t size) -> bool
{
  for (uint64_t i=0; i<size; i++)
  {
    if (buf[i] != next_sample())
      return false;
  }
  return true;
}

auto ska::pst::common::SineWaveGenerator::test_weights(char * buf, uint64_t size) -> bool
{
  return true;
}

auto ska::pst::common::SineWaveGenerator::test_scales(char * buf, uint64_t size) -> bool
{
  return true;
}

void ska::pst::common::SineWaveGenerator::reset ()
{
  current_sample = 0;
}

