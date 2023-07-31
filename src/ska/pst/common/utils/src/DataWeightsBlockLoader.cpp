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

#include "ska/pst/common/utils/DataWeightsBlockLoader.h"
#include <spdlog/spdlog.h>

#define LOG_THROW(x) SPDLOG_ERROR(x); throw std::runtime_error(x);

auto ska::pst::common::DataWeightsBlockLoader::get_data_header() const -> const AsciiHeader&
{
  SPDLOG_DEBUG("ska::pst::common::DataWeightsBlockLoader::get_data_header");
  if (!data_block_loader)
  {
    LOG_THROW("ska::pst::common::DataWeightsBlockLoader::get_data_header data_block_loader not initialised");
  }
  return data_block_loader->get_header();
}

auto ska::pst::common::DataWeightsBlockLoader::get_weights_header() const -> const AsciiHeader&
{
  SPDLOG_DEBUG("ska::pst::common::DataWeightsBlockLoader::get_weights_header");
  if (!weights_block_loader)
  {
    LOG_THROW("ska::pst::common::DataWeightsBlockLoader::get_weights_header weights_block_loader not initialised");
  }
  return weights_block_loader->get_header();
}


auto ska::pst::common::DataWeightsBlockLoader::next_block() -> Block
{
  SPDLOG_DEBUG("ska::pst::common::DataWeightsBlockLoader::next_block");

  if (!data_block_loader)
  {
    LOG_THROW("ska::pst::common::DataWeightsBlockLoader::next_block data_block_loader not initialised");
  }
  if (!weights_block_loader)
  {
    LOG_THROW("ska::pst::common::DataWeightsBlockLoader::next_block weights_block_loader not initialised");
  }

  Block result;
  result.data = data_block_loader->next_block();
  result.weights = weights_block_loader->next_block();
  return result;
}
