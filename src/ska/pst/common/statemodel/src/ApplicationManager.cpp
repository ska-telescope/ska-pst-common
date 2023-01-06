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
  spdlog::debug("ska::pst::common::ApplicationManager::main");
  wait_for(Initialise);
  perform_initialise();
}

void ska::pst::common::ApplicationManager::quit()
{
  spdlog::debug("ska::pst::common::ApplicationManager::quit set_command={}",get_name(Terminate));
  wait_for(Terminate);
  spdlog::debug("ska::pst::common::ApplicationManager::quit get_command={}",get_name(get_command()));
  perform_terminate();
}

void ska::pst::common::ApplicationManager::wait_for(ska::pst::common::Command cmd)
{
  spdlog::trace("ska::pst::common::ApplicationManager::wait_for command={}",get_name(cmd));
  std::unique_lock<std::mutex> control_lock(command_mutex);
  command_cond.wait(control_lock, [&]{return (command == cmd);});
  bool success = (command == cmd);
  spdlog::trace("ska::pst::common::ApplicationManager::wait_for state={} required={}",get_name(command) , get_name(cmd));
  control_lock.unlock();
  command_cond.notify_one();
  if (!success)
  {
    spdlog::debug("ska::pst::common::ApplicationManager::wait_for raise_exception()");
  }
  spdlog::trace("ska::pst::common::ApplicationManager::wait_for done");
}

void ska::pst::common::ApplicationManager::set_state(ska::pst::common::State required)
{
  spdlog::debug("ska::pst::common::ApplicationManager::set_state");
  previous_state=state;
  state=required;
}

void ska::pst::common::ApplicationManager::set_exception(std::exception exception)
{
  spdlog::debug("ska::pst::common::ApplicationManager::set_exception");
}

/*/ TBC: confirm if following protected methods are meant to be implemented by agents. i.e. smrb, recv, dsp
void ska::pst::common::ApplicationManager::perform_initialise()
{
  spdlog::debug("ska::pst::common::ApplicationManager::perform_initialise");
  set_state(Idle);
}
/*
void ska::pst::common::ApplicationManager::perform_terminate()
{
  spdlog::debug("ska::pst::common::ApplicationManager::perform_terminate");
  // TBA: Prerequisites prior for state to transition from Idle to Unknown
  wait_for(Terminate);
  set_state(Unknown);
}
*/
/*
void ska::pst::common::ApplicationManager::perform_configure_beam()
{
  spdlog::debug("ska::pst::common::ApplicationManager::perform_configure_beam");
  // TBA: Prerequisites prior for state to transition from Idle to BeamConfigured
  // TBA: Error handling transitioning to RuntimeError
  set_state(BeamConfigured);
}

void ska::pst::common::ApplicationManager::perform_configure_scan()
{
  spdlog::debug("ska::pst::common::ApplicationManager::perform_configure_scan");
  // TBA: Prerequisites prior for state to transition from BeamConfigured to ScanConfigured
  // TBA: Error handling transitioning to RuntimeError
  set_state(ScanConfigured);
}

void ska::pst::common::ApplicationManager::perform_scan()
{
  spdlog::debug("ska::pst::common::ApplicationManager::perform_scan");
  // TBA: Prerequisites prior for state to transition from ScanConfigured to Scanning
  // TBA: Error handling transitioning to RuntimeError
  set_state(Scanning);
}

void ska::pst::common::ApplicationManager::perform_reset()
{
  spdlog::debug("ska::pst::common::ApplicationManager::reset");
  // TBA: Prerequisites prior for state to transition from RuntimeError to Idle
  set_state(Idle);
}
*/
