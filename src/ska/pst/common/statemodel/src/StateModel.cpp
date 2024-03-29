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
#include "ska/pst/common/statemodel/StateModelException.h"

ska::pst::common::StateModel::StateModel()
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::StateModel()");
}

void ska::pst::common::StateModel::initialise()
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::initialise()");
  set_command(Initialise);
  wait_for_state(Idle);
}

void ska::pst::common::StateModel::configure_beam(const AsciiHeader& config)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::configure_beam()");

  // enforce valid request. This ensures the validation had happened before
  // attempting to perform configure beam on the service.
  ValidationContext validation_context;
  validate_configure_beam(config, &validation_context);
  validation_context.throw_error_if_not_empty();

  set_beam_config(config);
  set_command(ConfigureBeam);
  wait_for_state(BeamConfigured);
}

void ska::pst::common::StateModel::configure_scan(const AsciiHeader& config)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::configure_scan()");

  // enforce valid request. This ensures the validation had happened before
  // attempting to perform configure scan on the service.
  ValidationContext validation_context;
  validate_configure_scan(config, &validation_context);
  validation_context.throw_error_if_not_empty();

  set_scan_config(config);
  set_command(ConfigureScan);
  wait_for_state(ScanConfigured);
}

void ska::pst::common::StateModel::start_scan(const AsciiHeader& config)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::start_scan()");
  validate_start_scan(config);
  set_startscan_config(config);
  set_command(StartScan);
  wait_for_state(Scanning);
}

void ska::pst::common::StateModel::stop_scan()
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::stop_scan()");
  set_command(StopScan);
  wait_for_state(ScanConfigured);
}

void ska::pst::common::StateModel::deconfigure_scan()
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::deconfigure_scan()");
  set_command(DeconfigureScan);
  wait_for_state(BeamConfigured);
}

void ska::pst::common::StateModel::deconfigure_beam()
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::deconfigure_beam()");
  set_command(DeconfigureBeam);
  wait_for_state(Idle);
}

void ska::pst::common::StateModel::reset()
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::reset()");
  set_command(Reset);
  wait_for_state_without_error(Idle);
  SPDLOG_DEBUG("ska::pst::common::StateModel::reset() state={}", get_name(state));
}

void ska::pst::common::StateModel::set_command(Command required_cmd)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::set_command() required_cmd={}", get_name(required_cmd));

  ska::pst::common::Command cmd = required_cmd;
  {
    std::unique_lock<std::mutex> control_lock(command_mutex);
    // State not found
    if (allowed_commands.find(state) == allowed_commands.end())
    {
      std::string error_msg = "ska::pst::common::StateModel::set_command state=" + get_name(state) + " cmd=" + get_name(cmd) + " state did not exist in allowed_commands";
      throw ska::pst::common::pst_state_transition_error(error_msg);
    }

    // check if the specified command exists in the current state
    auto it = std::find(allowed_commands[state].begin(), allowed_commands[state].end(), cmd);
    bool allowed = (it != allowed_commands[state].end());
    if (!allowed)
    {
      std::string error_msg = "ska::pst::common::StateModel::set_command cmd=" + get_name(cmd) + " was not allowed for state=" + get_name(state);
      throw ska::pst::common::pst_state_transition_error(error_msg);
    }
    else
    {
      SPDLOG_DEBUG("ska::pst::common::StateModel::set_command command updated cmd={}", get_name(cmd));
      command = cmd;
    }
  }
  command_cond.notify_one();
}

void ska::pst::common::StateModel::wait_for_state(ska::pst::common::State required_state)
{
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state state={} required={}",state_names[state] , state_names[required_state]);

  ska::pst::common::State state_required = required_state;
  {
    std::unique_lock<std::mutex> control_lock(state_mutex);
    state_cond.wait(control_lock, [&]{return (state == state_required || state == ska::pst::common::RuntimeError);});
    bool success = (state == state_required);
    SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state state={} required={}",state_names[state] , state_names[state_required]);
    state_cond.notify_one();
    if (!success)
    {
      SPDLOG_DEBUG("ska::pst::common::StateModel::wait_for_state raise_exception()");
      raise_exception();
    }
  }
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state done");
}

void ska::pst::common::StateModel::wait_for_state_without_error(ska::pst::common::State required_state)
{
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state_without_error state={} required={}",state_names[state] , state_names[required_state]);

  ska::pst::common::State state_required = required_state;
  {
    std::unique_lock<std::mutex> control_lock(state_mutex);
    state_cond.wait(control_lock, [&]{return (state == state_required);});
    SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state_without_error state={} required={}",state_names[state] , state_names[state_required]);
    state_cond.notify_one();
  }
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state_without_error done");
}

auto ska::pst::common::StateModel::wait_for_state(ska::pst::common::State required, unsigned milliseconds) -> bool
{
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state required={} timeout={}", state_names[required], milliseconds);
  using namespace std::chrono_literals;
  std::chrono::milliseconds timeout = milliseconds * 1ms;
  std::unique_lock<std::mutex> control_lock(state_mutex);
  bool reached_required = state_cond.wait_for(control_lock, timeout, [&]{return (state == required);});
  control_lock.unlock();
  state_cond.notify_one();
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state reach_required={}", reached_required);
  return reached_required;
}

auto ska::pst::common::StateModel::wait_for_not_state(ska::pst::common::State required, unsigned milliseconds) -> bool
{
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_not_state required={} timeout={}", state_names[required], milliseconds);
  using namespace std::chrono_literals;
  std::chrono::milliseconds timeout = milliseconds * 1ms;
  std::unique_lock<std::mutex> control_lock(state_mutex);
  bool left_required = state_cond.wait_for(control_lock, timeout, [&]{return (state != required);});
  control_lock.unlock();
  state_cond.notify_one();
  SPDLOG_TRACE("ska::pst::common::StateModel::wait_for_state left_required={}", left_required);
  return left_required;
}

void ska::pst::common::StateModel::raise_exception()
{
  std::exception_ptr to_throw = last_exception;
  last_exception = nullptr;
  std::rethrow_exception(to_throw);
}

void ska::pst::common::StateModel::set_beam_config(const AsciiHeader &config)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::set_beam_config");
  beam_config.clone(config);
}

void ska::pst::common::StateModel::set_scan_config(const AsciiHeader &config)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::set_scan_config done");
  scan_config.clone(beam_config);
  scan_config.append_header(config);
}

void ska::pst::common::StateModel::set_startscan_config(const AsciiHeader &config)
{
  SPDLOG_DEBUG("ska::pst::common::StateModel::set_startscan_config done");
  startscan_config.clone(config);
}
