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

#include "ska/pst/common/utils/DataGenerator.h"

void ska::pst::common::DataGenerator::configure(const ska::pst::common::AsciiHeader& /* config */)
{
  spdlog::debug("ska::pst::common::DataGenerator::configure does nothing");
}

auto ska::pst::common::DataGenerator::test_block (char * buf) -> bool
{
  if (!layout_configured)
    throw std::runtime_error("ska::pst::common::DataGenerator::test_block block layout not configured");

  return test_weights(buf + layout.get_packet_weights_offset(), layout.get_packet_weights_size()) // NOLINT
     &&  test_data(buf + layout.get_packet_data_offset(), layout.get_packet_data_size()) // NOLINT
     &&  test_scales(buf + layout.get_packet_scales_offset(), layout.get_packet_scales_size()); // NOLINT
}

void ska::pst::common::DataGenerator::fill_block (char * buf)
{
  if (!layout_configured)
    throw std::runtime_error("ska::pst::common::DataGenerator::fill_block block layout not configured");

  fill_weights(buf + layout.get_packet_weights_offset(), layout.get_packet_weights_size()); // NOLINT
  fill_data(buf + layout.get_packet_data_offset(), layout.get_packet_data_size()); // NOLINT
  fill_scales(buf + layout.get_packet_scales_offset(), layout.get_packet_scales_size()); // NOLINT
}

void ska::pst::common::DataGenerator::copy_layout (const DataLayout* _layout)
{
  layout = *_layout;

  spdlog::debug("ska::pst::common::DataGenerator::configure_format packet_weights_size={}", layout.get_packet_weights_size());
  spdlog::debug("ska::pst::common::DataGenerator::configure_format packet_weights_offset={}", layout.get_packet_weights_offset());

  spdlog::debug("ska::pst::common::DataGenerator::configure_format packet_scales_size={}", layout.get_packet_scales_size());
  spdlog::debug("ska::pst::common::DataGenerator::configure_format packet_scales_offset={}", layout.get_packet_scales_offset());

  spdlog::debug("ska::pst::common::DataGenerator::configure_format packet_data_size={}", layout.get_packet_data_size());
  spdlog::debug("ska::pst::common::DataGenerator::configure_format packet_data_offset={}", layout.get_packet_data_offset());

  layout_configured = true;
}
