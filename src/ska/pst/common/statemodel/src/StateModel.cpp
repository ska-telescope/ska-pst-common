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


#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <spdlog/spdlog.h>

#include "ska/pst/common/statemodel/StateModel.h"
#include "ska/pst/common/utils/AsciiHeader.h"

ska::pst::common::StateModel::StateModel()
{
}

void ska::pst::common::StateModel::configure_beam()
{
  spdlog::debug("ska::pst::common::StateModel::configure_beam()");
}

void ska::pst::common::StateModel::configure_scan()
{
  spdlog::debug("ska::pst::common::StateModel::configure_scan()");
}

void ska::pst::common::StateModel::start_scan()
{
  spdlog::debug("ska::pst::common::StateModel::start_scan()"); 
}

void ska::pst::common::StateModel::stop_scan()
{
  spdlog::debug("ska::pst::common::StateModel::stop_scan()"); 
}

void ska::pst::common::StateModel::deconfigure_scan()
{
  spdlog::debug("ska::pst::common::StateModel::deconfigure_scan()");
}

void ska::pst::common::StateModel::deconfigure_beam()
{
  spdlog::debug("ska::pst::common::StateModel::deconfigure_beam()");
}

void ska::pst::common::StateModel::reset()
{
  spdlog::debug("ska::pst::common::StateModel::reset()");
}


void ska::pst::common::StateModel::validate_configure_beam(ska::pst::common::AsciiHeader config)
{
  spdlog::debug("ska::pst::common::StateModel::validate_configure_beam()");
}


void ska::pst::common::StateModel::validate_configure_scan(ska::pst::common::AsciiHeader config)
{
  spdlog::debug("ska::pst::common::StateModel::validate_configure_scan()");
}


void ska::pst::common::StateModel::validate_start_scan(ska::pst::common::AsciiHeader config)
{
  spdlog::debug("ska::pst::common::StateModel::validate_start_scan()");
}


void ska::pst::common::StateModel::set_command(ska::pst::common::Command cmd)
{
  spdlog::debug("ska::pst::common::StateModel::set_command()");
}


void ska::pst::common::StateModel::wait_for(ska::pst::common::State state)
{
  spdlog::debug("ska::pst::common::StateModel::wait_for()");
}