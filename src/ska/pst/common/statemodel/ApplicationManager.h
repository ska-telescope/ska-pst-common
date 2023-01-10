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

#include <iostream>
#include <vector>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/statemodel/StateModel.h"

#ifndef SKA_PST_COMMON_ApplicationManager_h
#define SKA_PST_COMMON_ApplicationManager_h

namespace ska {
namespace pst {
namespace common {
  /**
   * @brief The ApplicationManager
   * 
   */
  class ApplicationManager : public StateModel
  {
    public:
      /**
       * @brief Construct a new ApplicationManager object, initialising the State to Unknown.
       *
       */
      ApplicationManager(const std::string& entity);

      /**
       * @brief Destroy the ApplicationManager object
       *
       */
      ~ApplicationManager();

      /**
       * @brief Main thread of control for the implementing class, and uses UpdateCommands to transistion intermediate states to steady states.
       * Intended to be called in a separate std::thread from the LmcService that controls this state.
       *
       */
      void main();

      /**
       * @brief Issue the commands on the ApplicationManager required to terminate the main method.
       *
       */
      void quit();

    protected:
      /**
       * @brief Initialisation callback to be implemented in the child class.
       * The method should wait for the Unknown state and transition the state model to the Initialised
       *
       */
      virtual void perform_initialise() = 0;

      /**
       * @brief Beam configuration callback that is called by \ref main to transition the state from Idle to BeamConfigured.
       * 
       */
      virtual void perform_configure_beam() = 0;

      /**
       * @brief Scan configuration callback that is called by \ref main to transition the state from BeamConfigured to ScanConfigured.
       *
       */
      virtual void perform_configure_scan() = 0;

      /**
       * @brief Scan callback that is called by \ref main to spawn a child thread that calls perform_start_scan.
       * This method is expected to block until the scan is complete.
       *
       */
      virtual void perform_scan() = 0;
  
      /**
       * @brief Scan callback that is called by \ref main to transition the state from StartingScan to Scanning.
       * This method is expected to block until the scan is complete.
       *
       */
      virtual void perform_start_scan() = 0;

      /**
       * @brief StopScan callback that is called by \ref main to transition the state from Scanning to ScanConfigured.
       * This method is expected to block until state transitions to ScanConfigured
       * 
       */
      virtual void perform_stop_scan() = 0;

      /**
       * @brief Scan callback that is called by \ref main to transition the state from ScanConfigured to BeamConfigured.
       * This method is expected to block until state transitions to BeamConfigured
       * 
       */
    
      virtual void perform_deconfigure_scan() = 0;
      /**
       * @brief Scan callback that is called by \ref main to transition the state from BeamConfigured to Idle.
       * This method is expected to block until state transitions to Idle
       * 
       */
      virtual void perform_deconfigure_beam() = 0;

      /**
       * @brief Reset callback that is called by \ref main to transition the state from RuntimeError to Idle.
       * This method is expected to block until state transitions to Idle
       *
       */
      virtual void perform_reset() = 0;
      
      /**
       * @brief Terminate callback that is called by \ref main to transition the state from Idle to Terminating.
       * 
       */
      virtual void perform_terminate() = 0;

      /**
       * @brief Transition the state.
       *
       * @param required state to transition.
       */
      void set_state(State state);

    private:

      /**
       * @brief Wait for the command to complete.
       *
       * @param required command to wait for.
       * TBD
       */
      ska::pst::common::Command wait_for_command();


      /**
       * @brief Transition the state.
       *
       * @param required state to transition.
       */
      void set_exception(std::exception exception);

      //! Main thread of execution for the state model interface
      std::unique_ptr<std::thread> main_thread{nullptr};

      //! Scan thread of execution for the state model interface
      std::unique_ptr<std::thread> scan_thread{nullptr};

      //! Name of the agent using the ApplicationManager
      std::string entity;

      //! Previous state of the entity
      State previous_state;
  };

} // common
} // pst
} // ska

#endif