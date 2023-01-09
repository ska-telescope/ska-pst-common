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

#ifndef SKA_PST_COMMON_StateModel_h
#define SKA_PST_COMMON_StateModel_h

namespace ska {
namespace pst {
namespace common {
  /**
   * @brief Enumeration of states in the state model
   *
   */
  enum State {
    Unknown,
    Idle,
    BeamConfigured,
    ScanConfigured,
    Scanning,
    RuntimeError,
  };

  /**
   * @brief Mapping of enumerated states to strings that describe the state
   *
   */
  static std::map<State, std::string> state_names {
    { Unknown, "Unknown" },
    { Idle, "Idle" },
    { BeamConfigured, "Beam Configured" },
    { ScanConfigured, "Scan Configured", },
    { Scanning, "Scanning", },
    { RuntimeError, "Runtime Error" },
  };

  /**
   * @brief Enumeration of the commands that can be issued by the LmcService to
   * effect a change of state in the Receiver.
   *
   */
  enum Command {
    None,
    Initialise,
    ConfigureBeam,
    ConfigureScan,
    StartScan,
    StopScan,
    DeconfigureScan,
    DeconfigureBeam,
    Terminate,
    Reset,
  };

  /**
   * @brief Mapping of control commands to strings that describe the control command.
   *
   */
  static std::map<Command, std::string> command_names {
    { None, "None" },
    { Initialise, "Initialise" },
    { ConfigureBeam, "Configure Beam" },
    { ConfigureScan, "Configure Scan" },
    { StartScan, "Start Scan" },
    { StopScan, "Stop Scan" },
    { DeconfigureScan, "Deconfigure Scan" },
    { DeconfigureBeam, "Deconfigure Beam" },
    { Terminate, "Terminate" },
    { Reset, "Reset" }
  };

  /**
   * @brief Mapping of the list of Commands that are valid for a specified state.
   * If the state does not exist in this mapping, then it has no valid Command that
   * can be applied to it.
   *
   */
  static std::map<State, std::vector<Command>> allowed_commands {
    { Unknown, std::vector<Command> { Initialise } },
    { Idle, std::vector<Command> { ConfigureBeam, Terminate } },
    { BeamConfigured, std::vector<Command> { ConfigureScan, DeconfigureBeam } },
    { ScanConfigured, std::vector<Command> { StartScan, DeconfigureScan } },
    { Scanning, std::vector<Command> { StopScan } },
    { RuntimeError, std::vector<Command> { Reset } },
  };

  /**
   * @brief The state model that regulates the behaviour of the classes that inherit from it.
   * This model expects that commands will be executed on it from different threads
   *  - Commands: these will be issued by an LmcService, and commence a state change from a steady state (e.g. Idle) to an intermediate state (e.g. ConfiguringBeam).
   *
   */
  class StateModel {

    public:
      /**
       * @brief Construct a new StateModel object, initialising the State to Unknown.
       *
       */
      StateModel();

      /**
       * @brief Destroy the StateModel object
       *
       */
      virtual ~StateModel() = default;

      /**
       * @brief Issues the ConfigureBeam command and waits for the BeamConfigured or ConfiguringBeamError state to be reached.
       *
       */
      virtual void configure_beam(const AsciiHeader& config);

      /**
       * @brief Issues the ConfigureScan command and waits for the ScanConfigured or ConfiguringScanError state to be reached.
       *
       */
      virtual void configure_scan(const AsciiHeader& config);

      /**
       * @brief Issues the StartScan command and waits for the Scanning or ScanningError state to be reached.
       *
       */
      virtual void start_scan(const AsciiHeader& config);

      /**
       * @brief Issues the StopScan command and waits for the ScanConfigured or ScanningError state to be reached.
       *
       */
      virtual void stop_scan();

      /**
       * @brief Issues the DeconfigureScan command and waits for the BeamConfigured or ConfiguringBeamError state to be reached.
       *
       */
      virtual void deconfigure_scan();

      /**
       * @brief Issues the DeconfigureBeam command and waits for the Idle state to be reached.
       *
       */
      virtual void deconfigure_beam();

      /**
       * @brief Issues the Reset command and waits for the state to transition from RuntimeError to Idle
       *
       */
      virtual void reset();

      /**
       * @brief Return the current state of the state model
       *
       * @return State current state of the state model
       * 
       */
      State get_state() { return state; }

      /**
       * @brief Return a pointer to the most recently received exception.
       *
       * @return std::exception_ptr
       */
      std::exception_ptr get_exception() { return last_exception; };

      /**
       * @brief Return the current command of the state model
       *
       * @return Command current command of the state model
       * 
       */
      Command get_command() { return command; };

      /**
       * @brief Return the name of the specified  command.
       *
       * @param command command whose name to return
       * @return std::string name of the command
       * 
       */
      std::string get_name(Command command) { return command_names[command]; };

      /**
       * @brief Return the name of the specified state.
       *
       * @param state state whose name to return
       * @return std::string name of the state
       * 
       */
      std::string get_name(State state) { return state_names[state]; }

    protected:

      virtual void validate_configure_beam(const AsciiHeader& config) = 0;

      virtual void validate_configure_scan(const AsciiHeader& config) = 0;

      virtual void validate_start_scan(const AsciiHeader& config) = 0;

      /**
       * @brief Set the command used as a reference for transitioning between states.
       *
       * @param command command to be set.
       * TBD
       */
      void set_command(Command command);

      /**
       * @brief Wait for the state model to transition to the expected state or the error state.
       * If the state model transitions to the error state, the exception raised by the Receiver
       * that caused the error will be raised here.
       *
       * @param expected expected state to wait for.
       */
      void wait_for_state(State expected);

      //! Command variable
      Command command{None};

      //! Current state of the state model
      State state{Unknown};

      //! Reference to the most recently experienced exception
      std::exception_ptr last_exception{nullptr};

      //! Control access to the state attribute
      std::mutex state_mutex;

      //! Coordinates interactions with processes waiting on changes to the state attribute
      std::condition_variable state_cond;

      //! Control access to the command attribute
      std::mutex command_mutex;

      //! Coordinates interactions with processes waiting on changes to the command attribute
      std::condition_variable command_cond;

    private:


  };

} // common
} // pst
} // ska

#endif // SKA_PST_COMMON_StateModel_h