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
#include <utility>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/DataGenerator.h"

ska::pst::common::DataGenerator::DataGenerator(std::shared_ptr<ska::pst::common::DataLayout> _layout) :
  layout(std::move(std::move(_layout)))
{
}

void ska::pst::common::DataGenerator::configure(const ska::pst::common::AsciiHeader& config)
{
  nbit = config.get_uint32("NBIT");
  ndim = config.get_uint32("NDIM");
  npol = config.get_uint32("NPOL");
  nchan = config.get_uint32("NCHAN");
  SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::configure nchan={} ndim={} npol={} nbit={}", nchan, ndim, npol, nbit);

  if (ndim != 2)
  {
    SPDLOG_ERROR("ska::pst::common::DataGenerator::configure expected NDIM=2, but found {}", ndim);
    throw std::runtime_error("ska::pst::common::DataGenerator::configure expected valud of NDIM");
  }

  if (npol != 2)
  {
    SPDLOG_ERROR("ska::pst::common::DataGenerator::configure expected NPOL=2, but found {}", npol);
    throw std::runtime_error("ska::pst::common::DataGenerator::configure expected valud of NPOL");
  }

  if (nchan % layout->get_nchan_per_packet() != 0)
  {
    SPDLOG_ERROR("ska::pst::common::SineWaveGenerator::configure NCHAN={} was not a multiple of nchan_per_packet={}", nchan, layout->get_nchan_per_packet());
    throw std::runtime_error("ska::pst::common::SineWaveGenerator::configure invalid NCHAN");
  }
}

auto ska::pst::common::DataGenerator::test_packet(char * buf) -> bool
{
  SPDLOG_TRACE("ska::pst::common::DataGenerator::test_packet");
  return test_scales(buf + layout->get_packet_scales_offset(), layout->get_packet_scales_size()) // NOLINT
     &&  test_weights(buf + layout->get_packet_weights_offset(), layout->get_packet_weights_size()) // NOLINT
     &&  test_data(buf + layout->get_packet_data_offset(), layout->get_packet_data_size()); // NOLINT
}

void ska::pst::common::DataGenerator::fill_packet(char * buf)
{
  SPDLOG_TRACE("ska::pst::common::DataGenerator::fill_packet");
  fill_scales(buf + layout->get_packet_scales_offset(), layout->get_packet_scales_size()); // NOLINT
  fill_weights(buf + layout->get_packet_weights_offset(), layout->get_packet_weights_size()); // NOLINT
  fill_data(buf + layout->get_packet_data_offset(), layout->get_packet_data_size()); // NOLINT
}
