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
  spdlog::debug("ska::pst::common::StateModel::StateModel()");
}

void ska::pst::common::StateModel::configure_beam(const AsciiHeader& config)
{
  spdlog::debug("ska::pst::common::StateModel::configure_beam()");
  validate_configure_beam(config);
  set_command(ConfigureBeam);
  wait_for(BeamConfigured);
}

void ska::pst::common::StateModel::configure_scan(const AsciiHeader& config)
{
  spdlog::debug("ska::pst::common::StateModel::configure_scan()");
  validate_configure_scan(config);
  set_command(ConfigureScan);
  wait_for(ScanConfigured);
}

void ska::pst::common::StateModel::start_scan(const AsciiHeader& config)
{
  spdlog::debug("ska::pst::common::StateModel::start_scan()");
  validate_start_scan(config);
  set_command(StartScan);
  wait_for(Scanning);
}

void ska::pst::common::StateModel::stop_scan()
{
  spdlog::debug("ska::pst::common::StateModel::stop_scan()");
  set_command(StopScan);
  wait_for(ScanConfigured);
}

void ska::pst::common::StateModel::deconfigure_scan()
{
  spdlog::debug("ska::pst::common::StateModel::deconfigure_scan()");
  set_command(DeconfigureScan);
  wait_for(BeamConfigured);
}

void ska::pst::common::StateModel::deconfigure_beam()
{
  spdlog::debug("ska::pst::common::StateModel::deconfigure_beam()");
  set_command(DeconfigureBeam);
  wait_for(Idle);
}

void ska::pst::common::StateModel::reset()
{
  spdlog::debug("ska::pst::common::StateModel::reset()");
  set_command(Reset);
  wait_for(Idle);
}

void ska::pst::common::StateModel::set_command(Command cmd)
{
  spdlog::debug("ska::pst::common::StateModel::set_command() command={}", get_name(cmd));

  // State not found
  if (allowed_control_commands.find(state) == allowed_control_commands.end())
  {
    spdlog::warn("ska::pst::common::StateModel::set_command state={} did not exist in allowed_control_commands", state, get_name(cmd));
    spdlog::warn("ska::pst::common::StateModel::set_command state set to RuntimeError");
    state=RuntimeError;
  }

  // check if the specified command exists in the current state
  auto it = std::find(allowed_control_commands[state].begin(), allowed_control_commands[state].end(), cmd);
  bool allowed = (it != allowed_control_commands[state].end());
  if (!allowed)
  {
    spdlog::debug("ska::pst::common::StateModel::set_command cmd={} was not allowed for state={}", get_name(cmd), state_names[state]);
    throw std::runtime_error("ska::pst::common::StateModel::set_command was not allowed");
  }
  else 
  {
    spdlog::info("ska::pst::common::StateModel::set_command command updated cmd={}", get_name(cmd));
    command = cmd;
  }
}

void ska::pst::common::StateModel::wait_for(ska::pst::common::State required)
{
  spdlog::trace("ska::pst::common::StateModel::wait_for state={} required={}",state_names[state] , state_names[required]);
  std::unique_lock<std::mutex> control_lock(state_mutex);
  state_cond.wait(control_lock, [&]{return (state == required);});
  bool success = (state == required);
  spdlog::trace("ska::pst::common::StateModel::wait_for state={} required={}",state_names[state] , state_names[required]);
  control_lock.unlock();
  state_cond.notify_one();
  if (!success)
  {
    spdlog::debug("ska::pst::common::StateModel::wait_for raise_exception()");
  }
  spdlog::trace("ska::pst::common::StateModel::wait_for done");
}