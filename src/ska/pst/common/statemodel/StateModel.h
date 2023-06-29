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
#include "ska/pst/common/utils/ValidationContext.h"

#ifndef SKA_PST_COMMON_StateModel_h
#define SKA_PST_COMMON_StateModel_h

namespace ska::pst::common
{
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
       * @brief initialise call back responsible for the state transition from Unknown to Idle.
       * Required to be called by the Application that uses this statemodel in the Applications constructor class.
       * Calls the virtual method perform_initialise prior to the state transition.
       *
       */
      void initialise();

      /**
       * @brief Issues the ConfigureBeam command and waits for the BeamConfigured or ConfiguringBeamError state to be reached.
       *
       * @param config Beam configuration
       *
       */
      virtual void configure_beam(const AsciiHeader& config);

      /**
       * @brief Issues the ConfigureScan command and waits for the ScanConfigured or ConfiguringScanError state to be reached.
       *
       * @param config Scan configuration
       *
       */
      virtual void configure_scan(const AsciiHeader& config);

      /**
       * @brief Issues the StartScan command and waits for the Scanning or ScanningError state to be reached.
       *
       * @param config StartScan configuration
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
      State get_state() const { return state; }

      /**
       * @brief Return a pointer to the most recently received exception.
       *
       * @return std::exception_ptr
       */
      std::exception_ptr get_exception() { return last_exception; };

      /**
       * @brief Raise the stored exception from the ApplicationManager's main thread
       *
       */
      void raise_exception();

      /**
       * @brief Return the current command of the state model
       *
       * @return Command current command of the state model
       *
       */
      Command get_command() const { return command; };

      /**
       * @brief Return the name of the specified  command.
       *
       * @return std::string name of the command
       *
       */
      std::string get_name(Command command) const { return command_names[command]; };

      /**
       * @brief Return the name of the specified state.
       *
       * @param state state whose name to return
       * @return std::string name of the state
       *
       */
      std::string get_name(State state) const { return state_names[state]; }

      /**
       * @brief Get the beam configuration parameters.
       *
       * @return ska::pst::common::AsciiHeader& beam configuration parameters
       */
      virtual ska::pst::common::AsciiHeader& get_beam_configuration() {
        return beam_config;
      }

      /**
       * @brief Get the scan configuration parameters.
       *
       * @return ska::pst::common::AsciiHeader& scan configuration parameters
       */
      virtual ska::pst::common::AsciiHeader& get_scan_configuration() {
        return scan_config;
      }

      /**
       * @brief Get the start scan configuration parameters
       *
       * @return ska::pst::common::AsciiHeader& start scan configuration parameters
       */
      virtual ska::pst::common::AsciiHeader& get_startscan_configuration() {
        return startscan_config;
      }

      /**
       * @brief Validates Beam configuration.
       *
       * Validation errors should not be raise as exceptions but added to the
       * validation context.  The client of the method can decide what to do with
       * the validation errors.
       *
       * @param config Beam configuration to validate
       * @param context A validation context where errors should be added.
       */
      virtual void validate_configure_beam(const AsciiHeader& config, ValidationContext *context) = 0;

      /**
       * @brief Validates Scan configuration.
       *
       * Validation errors should not be raise as exceptions but added to the
       * validation context.  The client of the method can decide what to do with
       * the validation errors.
       *
       * @param config Scan configuration to validate
       * @param context A validation context where errors should be added.
       */
      virtual void validate_configure_scan(const AsciiHeader& config, ValidationContext *context) = 0;

      /**
       * @brief Validates StartScan configuration. Specific validation errors must be set when throwing exceptions.
       *
       * @param config StartScan configuration to validate
       *
       */
      virtual void validate_start_scan(const AsciiHeader& config) = 0;

    protected:
      /**
       * @brief Set the beam config object
       *
       * @param config
       */
      void set_beam_config(const AsciiHeader &config);

      /**
       * @brief Set the scan config object
       *
       * @param config
       */
      void set_scan_config(const AsciiHeader &config);

      /**
       * @brief Set the startscan config object
       *
       * @param config
       */
      void set_startscan_config(const AsciiHeader &config);

      /**
       * @brief Set the command used as a reference for transitioning between states.
       *
       * @param command command to be set.
       *
       */
      void set_command(Command command);

      /**
       * @brief Wait for the state model to transition to the required state or the RuntimeError state.
       * If the state model transitions to the RuntimeError state, the exception raised by the Application
       * that caused the error will be raised here.
       *
       * @param required state to wait for.
       *
       */
      void wait_for_state(State required);

      /**
       * @brief Wait for the state model to transition to the expected state.
       *
       * @param required state to wait for.
       *
       */
      void wait_for_state_without_error(State required);

      /**
       * @brief Wait for the state model to achieve the required state within the timeout
       *
       * @param expected target state for the state model to achieve.
       * @param milliseconds timeout in milliseconds.
       * @return true if the state model did achieve the required state before the timeout
       * @return false if the state model did not achieve the required state before the timeout
       */
      bool wait_for_state(State expected, unsigned milliseconds);

      /**
       * @brief Wait for the state model to achieve any state other than the required state, within the timeout.
       *
       * @param required the required state for the state model to
       * @param milliseconds timeout in milliseconds
       * @return true if the state model did achieve any state other than the required state within the timeout.
       * @return false if the state model did not achieve any state other than the required state within the timeout
       */
      bool wait_for_not_state(State required, unsigned milliseconds);

      //! Command variable
      Command command{None};

      //! Current state of the state model
      State state{Unknown};

      //! Beam configuration
      AsciiHeader beam_config;

      //! Scan configuration
      AsciiHeader scan_config;

      //! StartScan configuration
      AsciiHeader startscan_config;

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

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_StateModel_h