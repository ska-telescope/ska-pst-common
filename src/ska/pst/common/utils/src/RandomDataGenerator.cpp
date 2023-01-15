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

#include <cstring>
#include <cmath>
#include <stdexcept>
#include <spdlog/spdlog.h>

// #include "ska/pst/common/definitions.h"
#include "ska/pst/common/utils/RandomDataGenerator.h"

void ska::pst::common::RandomDataGenerator::configure(const ska::pst::common::AsciiHeader& config)
{
  spdlog::debug("ska::pst::common::RandomDataGenerator::configure");
  dat_sequence.configure(config);
  wts_sequence.configure(config);

#ifdef DEBUG
  dat_sequence.verbose = true;
#endif
}

auto ska::pst::common::RandomDataGenerator::test_data(char * buf, uint64_t size) -> bool
{
  return dat_sequence.validate(reinterpret_cast<uint8_t*> (buf), size);
}

auto ska::pst::common::RandomDataGenerator::test_weights(char * buf, uint64_t size) -> bool
{
  return wts_sequence.validate(reinterpret_cast<uint8_t*>(buf), size);
}

void ska::pst::common::RandomDataGenerator::fill_data_and_weights(char * buf)
{
  #ifdef DEBUG
  static unsigned print_count = 0;

  if (print_count < 2)
  {
    spdlog::debug("ska::pst::common::RandomDataGenerator::fill_data_and_weights packet_scales_size={}", packet_scales_size);
    spdlog::debug("ska::pst::common::RandomDataGenerator::fill_data_and_weights packet_scales_offset={}", packet_scales_offset);

    spdlog::debug("ska::pst::common::RandomDataGenerator::fill_data_and_weights packet_weights_size={}", packet_weights_size);
    spdlog::debug("ska::pst::common::RandomDataGenerator::fill_data_and_weights packet_weights_offset={}", packet_weights_offset);

    spdlog::debug("ska::pst::common::RandomDataGenerator::fill_data_and_weights packet_data_size={}", packet_data_size);
    spdlog::debug("ska::pst::common::RandomDataGenerator::fill_data_and_weights packet_data_offset={}", packet_data_offset);

    print_count ++;
  }
  #endif

  uint8_t* scales = reinterpret_cast<uint8_t*>(buf + packet_scales_offset); // NOLINT
  uint8_t* weights = reinterpret_cast<uint8_t*>(buf + packet_weights_offset); // NOLINT
  uint8_t* data = reinterpret_cast<uint8_t*>(buf + packet_data_offset); // NOLINT

  wts_sequence.generate(scales, packet_scales_size);
  wts_sequence.generate(weights, packet_weights_size);
  dat_sequence.generate(data, packet_data_size);
}
