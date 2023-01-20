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
#include "ska/pst/common/statemodel/ApplicationManager.h"

ska::pst::common::ApplicationManager::ApplicationManager(const std::string& _entity) : entity(_entity)
{
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::ApplicationManager({})", _entity);
  main_thread = std::make_unique<std::thread>(std::thread(&ska::pst::common::ApplicationManager::main, this));
}

ska::pst::common::ApplicationManager::~ApplicationManager()
{
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::~ApplicationManager");
  quit();
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::~ApplicationManager main_thread->join()");
  main_thread->join();
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::~ApplicationManager main_thread joined");
}

void ska::pst::common::ApplicationManager::main()
{
  std::string method_name = "ska::pst::common::ApplicationManager::main";
  SPDLOG_DEBUG("{}", method_name);

  SPDLOG_DEBUG("{} perform_initialise", method_name);
  perform_initialise();
  set_state(Idle);
  SPDLOG_DEBUG("{} perform_initialise done() state={}",method_name, state_names[get_state()]);

  // loop through the statemodel
  while(state != Unknown)
  {
    SPDLOG_DEBUG("{} [{}] state_model.wait_for_command", method_name, entity, entity);
    ska::pst::common::Command cmd = wait_for_command();
    SPDLOG_DEBUG("{} [{}] state={} command={}", method_name, entity, get_name(state), get_name(cmd));

    try {
      switch (cmd)
      {
        case ConfigureBeam:
          SPDLOG_TRACE("{} {} {} perform_configure_beam", method_name, entity, get_name(cmd));
          previous_state = Idle;
          perform_configure_beam();
          SPDLOG_TRACE("{} {} {} perform_configure_beam done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(BeamConfigured)", method_name, entity, get_name(cmd));
          set_state(BeamConfigured);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case ConfigureScan:
          SPDLOG_TRACE("{} {} {} perform_configure_scan", method_name, entity, get_name(cmd));
          previous_state = BeamConfigured;
          perform_configure_scan();
          SPDLOG_TRACE("{} {} {} perform_configure_scan done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(ScanConfigured)", method_name, entity, get_name(cmd));
          set_state(ScanConfigured);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case StartScan:
          SPDLOG_TRACE("{} {} {} perform_start_scan", method_name, entity, get_name(cmd));
          previous_state = ScanConfigured;
          perform_start_scan();
          scan_thread = std::make_unique<std::thread>(std::thread(&ska::pst::common::ApplicationManager::perform_scan, this));
          SPDLOG_TRACE("{} {} {} perform_start_scan done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(Scanning)", method_name, entity, get_name(cmd));
          set_state(Scanning);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case StopScan:
          SPDLOG_TRACE("{} {} {} perform_stop_scan", method_name, entity, get_name(cmd));
          previous_state = Scanning;
          perform_stop_scan();
          scan_thread->join();
          SPDLOG_TRACE("{} {} {} perform_stop_scan done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(ScanConfigured)", method_name, entity, get_name(cmd));
          set_state(ScanConfigured);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case DeconfigureScan:
          SPDLOG_TRACE("{} {} {} perform_deconfigure_scan", method_name, entity, get_name(cmd));
          previous_state = ScanConfigured;
          perform_deconfigure_scan();
          SPDLOG_TRACE("{} {} {} perform_deconfigure_scan done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(BeamConfigured)", method_name, entity, get_name(cmd));
          set_state(BeamConfigured);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;

        case DeconfigureBeam:
          SPDLOG_TRACE("{} {} {} perform_deconfigure_beam", method_name, entity, get_name(cmd));
          previous_state = BeamConfigured;
          perform_deconfigure_beam();
          SPDLOG_TRACE("{} {} {} perform_deconfigure_beam done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(Idle)", method_name, entity, get_name(cmd));
          set_state(Idle);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case Reset:
          SPDLOG_TRACE("{} {} {} perform_configure_beam", method_name, entity, get_name(cmd));
          perform_reset();
          SPDLOG_TRACE("{} {} {} perform_configure_beam done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(BeamConfigured)", method_name, entity, get_name(cmd));
          set_state(Idle);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;

        case Terminate:
          SPDLOG_TRACE("{} {} {} perform_terminate", method_name, entity, get_name(cmd));
          perform_terminate();
          SPDLOG_TRACE("{} {} {} perform_terminate done", method_name, entity, get_name(cmd));
          SPDLOG_TRACE("{} {} {} set_state(Unknown)", method_name, entity, get_name(cmd));
          set_state(Unknown);
          SPDLOG_TRACE("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
      }
    }
    catch (const std::exception& exc)
    {
      spdlog::warn("{} {} exception during command [{}] {}", method_name, entity, get_name(cmd), exc.what());
      set_exception(exc);
      SPDLOG_DEBUG("ska::pst::common::ApplicationManager::set_exception done");
      set_state(RuntimeError);
      SPDLOG_DEBUG("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
    }
  }
}

void ska::pst::common::ApplicationManager::quit()
{
  if (get_state() == Scanning)
  {
    set_command(StopScan);
    wait_for_state(ScanConfigured);
  }

  if (get_state() == ScanConfigured)
  {
    set_command(DeconfigureScan);
    wait_for_state(BeamConfigured);
  }

  if (get_state() == BeamConfigured)
  {
    set_command(DeconfigureBeam);
    wait_for_state(Idle);
  }

  if (get_state() == RuntimeError)
  {
    set_command(Reset);
    wait_for_state(Idle);
  }

  if (get_state() == Idle)
  {
    set_command(Terminate);
    wait_for_state(Unknown);
  }
}

ska::pst::common::Command ska::pst::common::ApplicationManager::wait_for_command()
{
  SPDLOG_TRACE("ska::pst::common::ApplicationManager::wait_for_command [{}] command={}", entity, command);

  ska::pst::common::Command cmd = command;
  {
    std::unique_lock<std::mutex> control_lock(command_mutex);
    command_cond.wait(control_lock, [&]{return (command != None);});
    cmd = command;
    command = None;
  }
  command_cond.notify_one();
  SPDLOG_TRACE("ska::pst::common::ApplicationManager::wait_for_command [{}] done", entity);
  return cmd;
}

void ska::pst::common::ApplicationManager::set_state(ska::pst::common::State new_state)
{
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::set_state({})", get_name(new_state));
  ska::pst::common::State state_required = new_state;
  {
    std::unique_lock<std::mutex> control_lock(state_mutex);
    if (entity.size() == 0)
    {
      SPDLOG_INFO("{} -> {}", get_name(state), get_name(state_required));
    }
    else
    {
      SPDLOG_DEBUG("{}: {} -> {}", entity, get_name(state), get_name(state_required));
    }
    previous_state = state;
    state = new_state;
    state_cond.notify_one();
  }
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::set_state done state={}", get_name(get_state()));
}

void ska::pst::common::ApplicationManager::set_exception(std::exception exception)
{
  SPDLOG_DEBUG("ska::pst::common::ApplicationManager::set_exception");
  last_exception = std::make_exception_ptr(exception);
}

ska::pst::common::State ska::pst::common::ApplicationManager::get_previous_state()
{
  return previous_state;
}