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
  spdlog::debug("ska::pst::common::ApplicationManager::ApplicationManager({})", _entity);
  main_thread = std::make_unique<std::thread>(std::thread(&ska::pst::common::ApplicationManager::main, this));
}

ska::pst::common::ApplicationManager::~ApplicationManager()
{
  spdlog::debug("ska::pst::common::ApplicationManager::~ApplicationManager");
  quit();
  spdlog::debug("ska::pst::common::ApplicationManager::~ApplicationManager main_thread->join()");
  main_thread->join();
  spdlog::debug("ska::pst::common::ApplicationManager::~ApplicationManager main_thread joined");
}

void ska::pst::common::ApplicationManager::main()
{
  std::string method_name = "ska::pst::common::ApplicationManager::main";
  spdlog::debug("{}", method_name);
  // wait_for_state(Initialise);
  spdlog::debug("{} perform_initialise", method_name);
  perform_initialise();
  set_state(Idle);
  spdlog::debug("{} perform_initialise done() state={}",method_name, state_names[get_state()]);

  // loop through the statemodel
  while(state != Unknown)
  {
    spdlog::debug("{} [{}] state_model.wait_for_command", method_name, entity, entity);
    ska::pst::common::Command cmd = wait_for_command();
    spdlog::debug("{} [{}] state={} command={}", method_name, entity, entity, get_name(state), get_name(cmd));

    try {
      switch (cmd)
      {
        case ConfigureBeam:
          spdlog::trace("{} {} {} perform_configure_beam", method_name, entity, get_name(cmd));
          perform_configure_beam();
          spdlog::trace("{} {} {} perform_configure_beam done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(BeamConfigured)", method_name, entity, get_name(cmd));
          set_state(BeamConfigured);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case ConfigureScan:
          spdlog::trace("{} {} {} perform_configure_scan", method_name, entity, get_name(cmd));
          perform_configure_scan();
          spdlog::trace("{} {} {} perform_configure_scan done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(ScanConfigured)", method_name, entity, get_name(cmd));
          set_state(ScanConfigured);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case StartScan:
          spdlog::trace("{} {} {} perform_start_scan", method_name, entity, get_name(cmd));
          perform_start_scan();
          scan_thread = std::make_unique<std::thread>(std::thread(&ska::pst::common::ApplicationManager::perform_scan, this));
          spdlog::trace("{} {} {} perform_start_scan done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(Scanning)", method_name, entity, get_name(cmd));
          set_state(Scanning);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case StopScan:
          spdlog::trace("{} {} {} perform_stop_scan", method_name, entity, get_name(cmd));
          perform_stop_scan();
          scan_thread->join();
          spdlog::trace("{} {} {} perform_stop_scan done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(ScanConfigured)", method_name, entity, get_name(cmd));
          set_state(ScanConfigured);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case DeconfigureScan:
          spdlog::trace("{} {} {} perform_deconfigure_scan", method_name, entity, get_name(cmd));
          perform_deconfigure_scan();
          spdlog::trace("{} {} {} perform_deconfigure_scan done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(BeamConfigured)", method_name, entity, get_name(cmd));
          set_state(BeamConfigured);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;

        case DeconfigureBeam:
          spdlog::trace("{} {} {} perform_deconfigure_beam", method_name, entity, get_name(cmd));
          perform_deconfigure_beam();
          spdlog::trace("{} {} {} perform_deconfigure_beam done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(Idle)", method_name, entity, get_name(cmd));
          set_state(Idle);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
          
        case Reset:
          spdlog::trace("{} {} {} perform_configure_beam", method_name, entity, get_name(cmd));
          perform_reset();
          spdlog::trace("{} {} {} perform_configure_beam done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(BeamConfigured)", method_name, entity, get_name(cmd));
          set_state(Idle);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;

        case Terminate:
          spdlog::trace("{} {} {} perform_terminate", method_name, entity, get_name(cmd));
          perform_terminate();
          spdlog::trace("{} {} {} perform_terminate done", method_name, entity, get_name(cmd));
          spdlog::trace("{} {} {} set_state(Unknown)", method_name, entity, get_name(cmd));
          set_state(Unknown);
          spdlog::trace("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
          break;
      }
    }
    catch (const std::exception& exc)
    {
      spdlog::warn("{} {} exception during command [{}] {}", method_name, entity, get_name(cmd), exc.what());
      set_exception(exc);
      spdlog::debug("ska::pst::common::ApplicationManager::set_exception done");
      set_state(RuntimeError);
      spdlog::debug("{} {} [{}] state={}", method_name, entity, get_name(cmd), state_names[get_state()]);
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
  spdlog::trace("ska::pst::common::ApplicationManager::wait_for_command [{}] command={}", entity, command);

  ska::pst::common::Command cmd = command;
  {
    std::unique_lock<std::mutex> control_lock(command_mutex);
    command_cond.wait(control_lock, [&]{return (command != None);});
    cmd = command;
    command = None;
  }
  command_cond.notify_one();
  spdlog::trace("ska::pst::common::ApplicationManager::wait_for_command [{}] done", entity);
  return cmd;
}

void ska::pst::common::ApplicationManager::set_state(ska::pst::common::State new_state)
{
  spdlog::debug("ska::pst::common::ApplicationManager::set_state({})", get_name(new_state));
  ska::pst::common::State state_required = new_state;
  {
    std::unique_lock<std::mutex> control_lock(state_mutex);
    if (entity.size() == 0)
    {
      spdlog::info("{} -> {}", get_name(state), get_name(state_required));
    }
    else
    {
      spdlog::debug("{}: {} -> {}", entity, get_name(state), get_name(state_required));
    }
    previous_state = state;
    state = new_state;
    state_cond.notify_one();
  }
  spdlog::debug("ska::pst::common::ApplicationManager::set_state done state={}", get_name(get_state()));
}

void ska::pst::common::ApplicationManager::set_exception(std::exception exception)
{
  spdlog::debug("ska::pst::common::ApplicationManager::set_exception");
  last_exception = std::make_exception_ptr(exception);
}