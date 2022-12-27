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

ska::pst::common::StateModel::StateModel(const std::string& _entity) : entity(_entity)
{
}

void ska::pst::common::StateModel::main()
{
  spdlog::debug("ska::pst::common::StateModel::main perform_initialise()");
  perform_initialise();
  spdlog::debug("ska::pst::common::StateModel::main perform_initialise() done");

  std::vector<ska::pst::common::State> states {ConfiguringBeam, ConfiguringScan, StartingScan, StoppingScan, DeconfiguringScan, DeconfiguringBeam, Terminating };

  spdlog::debug("ska::pst::common::StateModel::main state={}", get_name(state));
  while (state != Terminating)
  {
    spdlog::debug("ska::pst::common::StateModel::main [{}] state_model.wait_for(states)", entity);
    state = wait_for(states);
    spdlog::debug("ska::pst::common::StateModel::main [{}] state=={}", entity, get_name(state));

    if (state == ConfiguringBeam)
    {
      try {
        spdlog::debug("ska::pst::common::StateModel::main [{}] perform_configure_beam()", entity);
        perform_configure_beam();
        execute(ConfigureBeamComplete);
      } catch (const std::exception& exc) {
        spdlog::warn("ska::pst::common::StateModel::main [{}] exception during perform_configure_beam: {}", entity, exc.what());
        set_exception(exc);
        execute(ConfigureBeamFailed);
      }
    }
    else if (state == ConfiguringScan)
    {
      try {
        spdlog::debug("ska::pst::common::StateModel::main [{}] perform_configure_scan()", entity);
        perform_configure_scan();
        execute(ConfigureScanComplete);
      } catch (const std::exception& exc) {
        spdlog::warn("ska::pst::common::StateModel::main [{}] exception during perform_configure_scan: {}", entity, exc.what());
        set_exception(exc);
        execute(ConfigureScanFailed);
      }
    }
    else if (state == StartingScan)
    {
      try {
        spdlog::debug("ska::pst::common::StateModel::main [{}] perform_scan()", entity);
        perform_scan();
        execute(ScanStopped);
      } catch (const std::exception& exc) {
        spdlog::warn("ska::pst::common::StateModel::main [{}] exception during perform_scan: {}", entity, exc.what());
        set_exception(exc);
        execute(ScanFailed);
      }
    }
    else if (state == StoppingScan)
    {
      ;
    }
    else if (state == DeconfiguringScan)
    {
      try {
        spdlog::debug("ska::pst::common::StateModel::main [{}] perform_deconfigure_scan()", entity);
        perform_deconfigure_scan();
        execute(DeconfigureScanComplete);
      } catch (const std::exception& exc) {
        set_exception(exc);
        execute(DeconfigureScanFailed);
      }
    }
    else if (state == DeconfiguringBeam)
    {
      try {
        spdlog::debug("ska::pst::common::StateModel::main [{}] perform_deconfigure_beam()", entity);
        perform_deconfigure_beam();
        execute(DeconfigureBeamComplete);
      } catch (const std::exception& exc) {
        set_exception(exc);
        execute(DeconfigureBeamFailed);
      }
    }
    else
    {
      spdlog::debug("ska::pst::common::StateModel::main [{}] Ignoring state={}", entity, get_name());
    }
  }

  spdlog::debug("ska::pst::common::StateModel::main [{}] perform_terminate()", entity);
  try {
    perform_terminate();
  } catch (const std::exception& exc) {
    spdlog::warn("ska::pst::common::StateModel::main [{}] perform_terminate threw an exception: {}", entity, exc.what());
  }

  spdlog::debug("ska::pst::common::StateModel::main [{}] completed", entity);
}

void ska::pst::common::StateModel::quit()
{
  spdlog::debug("ska::pst::common::StateModel::quit [{}]", entity);
  if (is_scanning())
  {
    spdlog::debug("ska::pst::common::StateModel::quit [{}] stopping scan", entity);
    execute(StopScan);
    wait_for(ScanConfigured, ScanningError);
  }
  if (is_scan_configured())
  {
    spdlog::debug("ska::pst::common::StateModel::quit [{}] deconfiguring scan", entity);
    execute(DeconfigureScan);
    wait_for(BeamConfigured, ConfiguringBeamError);
  }
  if (state == ConfiguringScanError)
  {
    spdlog::debug("ska::pst::common::StateModel::quit [{}] resetting configure scan error", entity);
    execute(ResetConfiguringScanError);
    wait_for(BeamConfigured, ConfiguringBeamError);
  }
  if (is_beam_configured())
  {
    spdlog::debug("ska::pst::common::StateModel::quit [{}] deconfiguring beam", entity);
    execute(DeconfigureBeam);
    wait_for(Idle);
  }
  if (state == ConfiguringBeamError)
  {
    spdlog::debug("ska::pst::common::StateModel::quit [{}] resetting configure beam error", entity);
    execute(ResetConfiguringBeamError);
    wait_for(Idle);
  }
  if (is_idle())
  {
    spdlog::debug("ska::pst::common::StateModel::quit [{}] terminating()", entity);
    execute(Terminate);
  }
}

void ska::pst::common::StateModel::configure_beam()
{
  execute(ConfigureBeam);
  wait_for(BeamConfigured, ConfiguringBeamError);
}

void ska::pst::common::StateModel::configure_scan()
{
  execute(ConfigureScan);
  wait_for(ScanConfigured, ConfiguringScanError);
}

void ska::pst::common::StateModel::start_scan()
{
  execute(StartScan);
  wait_for(Scanning, ScanningError);
}

void ska::pst::common::StateModel::stop_scan()
{
  execute(StopScan);
  wait_for(ScanConfigured, ScanningError);
}

void ska::pst::common::StateModel::deconfigure_scan()
{
  execute(DeconfigureScan);
  wait_for(BeamConfigured, ConfiguringBeamError);
}

void ska::pst::common::StateModel::deconfigure_beam()
{
  execute(DeconfigureBeam);
  wait_for(Idle);
}

void ska::pst::common::StateModel::reset_beam_configuration()
{
  execute(ResetConfiguringBeamError);
  wait_for(Idle);
}

void ska::pst::common::StateModel::reset_scan_configuration()
{
  execute(ResetConfiguringScanError);
  wait_for(BeamConfigured);
}

void ska::pst::common::StateModel::reset_scanning()
{
  execute(ResetScanningError);
  wait_for(ScanConfigured);
}

void ska::pst::common::StateModel::set_exception(const std::exception& exc)
{
  last_exception = std::make_exception_ptr(exc);
}

auto ska::pst::common::StateModel::get_exception() -> std::exception_ptr
{
  return last_exception;
}

void ska::pst::common::StateModel::raise_exception()
{
  std::exception_ptr to_throw = last_exception;
  last_exception = nullptr;
  std::rethrow_exception(to_throw);
}

auto ska::pst::common::StateModel::is_idle() -> bool
{
  return (state == Idle);
}

auto ska::pst::common::StateModel::is_beam_configured() const -> bool
{
  return (state == BeamConfigured || state == ConfiguringScan || state == DeconfiguringScan || is_scan_configured());
}

auto ska::pst::common::StateModel::is_scan_configured() const -> bool
{
  return (state == ScanConfigured || state == StartingScan || state == StoppingScan || is_scanning());
}

auto ska::pst::common::StateModel::is_scanning() const -> bool
{
  return (state == StartingScan || state == Scanning);
}

auto ska::pst::common::StateModel::allowed(ska::pst::common::ControlCommand cmd) -> bool
{
  if (allowed_control_commands.find(state) == allowed_control_commands.end())
  {
    spdlog::warn("ska::pst::common::StateModel::allowed [{}] state={} did not exist in allowed_control_commands", entity, get_name());
    return false;
  }

  // check if the specified command exists in the current state
  auto it = std::find(allowed_control_commands[state].begin(), allowed_control_commands[state].end(), cmd);
  bool allowed = (it != allowed_control_commands[state].end());
  if (!allowed)
  {
    spdlog::warn("ska::pst::common::StateModel::allowed [{}] cmd={} was not allowed for state={}", entity, get_name(cmd), get_name());
  }
  return allowed;
}

void ska::pst::common::StateModel::execute(ska::pst::common::ControlCommand cmd)
{
  std::unique_lock<std::mutex> control_lock(mutex);
  spdlog::trace("ska::pst::common::StateModel::execute [{}] allowed({})", entity, get_name(cmd));
  if (allowed(cmd))
  {
    if (entity.size() == 0)
    {
      spdlog::info("{} -> {}", get_name(), get_name(control_transitions[cmd]));
    }
    else
    {
      spdlog::debug("{}: {} -> {}", entity, get_name(), get_name(control_transitions[cmd]));
    }
    state = control_transitions[cmd];
    control_lock.unlock();
    cond.notify_one();
  }
  else
  {
    control_lock.unlock();
    throw std::runtime_error("ska::pst::common::StateModel::process_command could not accept command");
  }
}

auto ska::pst::common::StateModel::allowed(ska::pst::common::UpdateCommand cmd) -> bool
{
  if (allowed_update_commands.find(state) == allowed_update_commands.end())
  {
    spdlog::warn("ska::pst::common::StateModel::allowed cmd={}", get_name(cmd));
    spdlog::warn("ska::pst::common::StateModel::allowed state={} did not exist in allowed_update_commands", get_name());
    return false;
  }

  auto it = std::find(allowed_update_commands[state].begin(), allowed_update_commands[state].end(), cmd);
  bool allowed = (it != allowed_update_commands[state].end());
  if (!allowed)
  {
    spdlog::warn("ska::pst::common::StateModel::allowed cmd={} was not allowed for state={}", get_name(cmd), get_name());
  }
  return allowed;
}

void ska::pst::common::StateModel::execute(ska::pst::common::UpdateCommand cmd)
{
  spdlog::trace("ska::pst::common::StateModel::execute cmd={}", get_name(cmd));
  std::unique_lock<std::mutex> control_lock(mutex);
  if (allowed(cmd))
  {
    if (entity.size() == 0)
    {
      spdlog::info("{} -> {}", get_name(), get_name(update_transitions[cmd]));
    }
    else
    {
      spdlog::debug("{}: {} -> {}", entity, get_name(), get_name(update_transitions[cmd]));
    }
    state = update_transitions[cmd];
    control_lock.unlock();
    cond.notify_one();
  }
  else
  {
    control_lock.unlock();
    throw std::runtime_error("ska::pst::common::StateModel::execute could not accept command");
  }
}

auto ska::pst::common::StateModel::allowed(ska::pst::common::ResetCommand cmd) -> bool
{
  if (allowed_reset_commands.find(state) == allowed_reset_commands.end())
  {
    spdlog::warn("ska::pst::common::StateModel::allowed state={} did not exist in allowed_reset_commands", get_name());
    return false;
  }

  bool allowed = (cmd == allowed_reset_commands[state]);
  if (!allowed)
  {
    spdlog::warn("ska::pst::common::StateModel::allowed cmd={} was not allowed for state={}", get_name(cmd), get_name());
  }
  return allowed;
}

void ska::pst::common::StateModel::execute(ska::pst::common::ResetCommand cmd)
{
  spdlog::trace("ska::pst::common::StateModel::execute cmd={}", get_name(cmd));
  std::unique_lock<std::mutex> control_lock(mutex);
  if (allowed(cmd))
  {
    if (entity.size() == 0)
    {
      spdlog::info("{} -> {}", get_name(), get_name(reset_transitions[cmd]));
    }
    else
    {
      spdlog::debug("{}: {} -> {}", entity, get_name(), get_name(reset_transitions[cmd]));
    }
    state = reset_transitions[cmd];
    control_lock.unlock();
    cond.notify_one();
  }
  else
  {
    control_lock.unlock();
    throw std::runtime_error("ska::pst::common::StateModel::execute could not accept command");
  }
}

void ska::pst::common::StateModel::wait_for(ska::pst::common::State required)
{
  spdlog::trace("ska::pst::common::StateModel::wait_for [{}] current={} required={}", entity, get_name(), get_name(required));
  std::unique_lock<std::mutex> control_lock(mutex);
  cond.wait(control_lock, [&]{return (state == required);});
  bool success = (state == required);
  spdlog::trace("ska::pst::common::StateModel::wait_for [{}] state={} required={}", entity, get_name(state), get_name(required));
  control_lock.unlock();
  cond.notify_one();
  if (!success)
  {
    spdlog::debug("ska::pst::common::StateModel::wait_for [{}] raise_exception()", entity);
    raise_exception();
  }
  spdlog::trace("ska::pst::common::StateModel::wait_for [{}] done", entity);
}

void ska::pst::common::StateModel::wait_for(ska::pst::common::State expected, ska::pst::common::State error)
{
  spdlog::debug("ska::pst::common::StateModel::wait_for [{}] current={} expected={} error={}", entity, get_name(), get_name(expected), get_name(error));
  std::unique_lock<std::mutex> control_lock(mutex);
  cond.wait(control_lock, [&]{return (state == expected || state == error);});
  bool success = (state == expected);
  control_lock.unlock();
  cond.notify_one();
  if (!success)
  {
    spdlog::debug("ska::pst::common::StateModel::wait_for [{}] raise_expection()", entity);
    raise_exception();
  }
}

auto ska::pst::common::StateModel::wait_for(const std::vector<ska::pst::common::State>& states) -> ska::pst::common::State
{
  spdlog::debug("ska::pst::common::StateModel::wait_for [{}] states", entity);
  std::unique_lock<std::mutex> control_lock(mutex);
  cond.wait(control_lock, [&]{return std::find(states.begin(), states.end(), state) != states.end();});
  ska::pst::common::State reached = state;
  control_lock.unlock();
  cond.notify_one();
  return reached;
}

auto ska::pst::common::StateModel::wait_for(ska::pst::common::State required, unsigned milliseconds) -> bool
{
  using namespace std::chrono_literals;
  std::chrono::milliseconds timeout = milliseconds * 1ms;
  std::unique_lock<std::mutex> control_lock(mutex);
  bool reached_required = cond.wait_for(control_lock, timeout, [&]{return (state != required);});
  control_lock.unlock();
  cond.notify_one();
  return reached_required;
}

void ska::pst::common::StateModel::check_beam_configured(bool expected) const
{
  if (is_beam_configured() != expected)
  {
    spdlog::error("ska::pst::common::StateModel::check_beam_configured state {} did not match expected {}", is_beam_configured(), expected);
    throw std::runtime_error("ska::pst::common::StateModel::check_beam_configured state did not match expected value");
  }
}

void ska::pst::common::StateModel::check_scan_configured(bool expected) const
{
  if (is_scan_configured() != expected)
  {
    spdlog::error("ska::pst::common::StateModel::check_scan_configured state {} did not match expected {}", is_scan_configured(), expected);
    throw std::runtime_error("ska::pst::common::StateModel::check_scan_configured state did not match expected value");
  }
}

void ska::pst::common::StateModel::check_scanning(bool expected) const
{
  spdlog::trace("ska::pst::common::StateModel::check_scanning [{}] expected={} actual={}", entity, expected, is_scanning());
  if (is_scanning() != expected)
  {
    spdlog::error("ska::pst::common::StateModel::check_scanning [{}] state {} did not match expected {}", entity, is_scanning(), expected);
    throw std::runtime_error("ska::pst::common::StateModel::check_scanning state did not match expected value");
  }
}