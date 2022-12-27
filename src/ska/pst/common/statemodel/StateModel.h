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

#ifndef SKA_PST_COMMON_StateModel_h
#define SKA_PST_COMMON_StateModel_h

namespace ska {
namespace pst {
namespace common {

  /**
   * @brief Enumeration of states in the Receiver's state model
   *
   */
  enum State {
    Unknown,
    InitialisationError,
    Idle,
    ConfiguringBeam,
    ConfiguringBeamError,
    BeamConfigured,
    ConfiguringScan,
    ConfiguringScanError,
    ScanConfigured,
    StartingScan,
    Scanning,
    ScanningError,
    StoppingScan,
    DeconfiguringScan,
    DeconfiguringScanError,
    DeconfiguringBeam,
    DeconfiguringBeamError,
    Terminating,
  };

  /**
   * @brief Mapping of enumerated states to strings that describe the state
   *
   */
  static std::map<State, std::string> state_names {
    { Unknown, "Unknown" },
    { InitialisationError, "Initialisation Error" },
    { Idle, "Idle" },
    { ConfiguringBeam, "ConfiguringBeam", },
    { BeamConfigured, "Beam Configured" },
    { ConfiguringBeamError, "Configuring Beam Error" },
    { ConfiguringScan, "Configuring Scan" },
    { ScanConfigured, "ScanConfigured", },
    { ConfiguringScanError, "Configuring Scan Error" },
    { StartingScan, "Starting Scan" },
    { Scanning, "Scanning", },
    { ScanningError, "Scanning Error", },
    { StoppingScan,  "Stopping Scan" },
    { DeconfiguringScan, "Deconfiguring Scan" },
    { DeconfiguringScanError, "Deconfiguring Scan Error" },
    { DeconfiguringBeam, "Deconfiguring Beam" },
    { DeconfiguringBeamError, "Deconfiguring Beam Error" },
    { Terminating, "Terminating" },
  };

  /**
   * @brief Enumeration of the control commands that can be issued by the LmcService to
   * effect a change of state in the Receiver.
   *
   */
  enum ControlCommand {
    ConfigureBeam,
    ConfigureScan,
    StartScan,
    StopScan,
    DeconfigureScan,
    DeconfigureBeam,
    Terminate
  };

  /**
   * @brief Mapping of control commands to strings that describe the control command.
   *
   */
  static std::map<ControlCommand, std::string> control_command_names {
    { ConfigureBeam, "Configure Beam" },
    { ConfigureScan, "ConfigureScan" },
    { StartScan, "Start Scan" },
    { StopScan, "Stop Scan" },
    { DeconfigureScan, "Deconfigure Scan" },
    { DeconfigureBeam, "Deconfigure Beam" },
    { Terminate, "Terminate" }
  };

  /**
   * @brief Enumeration of update commands that the Receiver will use to change the
   * State of the receiver after receiving a control command.
   *
   */
  enum UpdateCommand {
    InitialisationComplete,
    InitialisationFailed,
    ConfigureBeamComplete,
    ConfigureBeamFailed,
    ConfigureScanComplete,
    ConfigureScanFailed,
    ScanStarted,
    ScanFailed,
    ScanStopped,
    DeconfigureScanComplete,
    DeconfigureScanFailed,
    DeconfigureBeamComplete,
    DeconfigureBeamFailed
  };

  /**
   * @brief Mapping of update commands to strings that describe the update command.
   *
   */
  static std::map<UpdateCommand, std::string> update_command_names {
    { InitialisationComplete, "Initialisation Complete" },
    { InitialisationFailed, "Initialisation Failed" },
    { ConfigureBeamComplete, "Configure Beam Complete" },
    { ConfigureBeamFailed, "Configure Beam Failed" },
    { ConfigureScanComplete, "Configure Scan Complete" },
    { ConfigureScanFailed, "Configure Scan Failed" },
    { ScanStarted, "Scan Started" },
    { ScanFailed, "Scan Failed" },
    { ScanStopped, "Scan Stopped" },
    { DeconfigureScanComplete, "Deconfigure Scan Complete" },
    { DeconfigureScanFailed, "Deconfigure Scan Failed" },
    { DeconfigureBeamComplete, "Deconfigure Beam Complete" },
    { DeconfigureBeamFailed, "Deconfigure Beam Failed" }
  };

  /**
   * @brief Enumeration of reset commands that the LmcService can use to reset the
   * Receiver when it has transitioned to an error state.
   *
   */
  enum ResetCommand {
    ResetConfiguringBeamError,
    ResetConfiguringScanError,
    ResetScanningError,
  };

  /**
   * @brief Mapping from reset commands to a string describing the reset command.
   *
   */
  static std::map<ResetCommand, std::string> reset_command_names {
    { ResetConfiguringBeamError, "Reset Configuring Beam Error" },
    { ResetConfiguringScanError, "Reset Configuring Scan Error" },
    { ResetScanningError, "Reset ConfiguringScan Error" },
  };

  /**
   * @brief Mapping of the list of ControlCommands that are valid for a specified state.
   * If the state does not exist in this mapping, then it has no valid ControlCommand that
   * can be applied to it.
   *
   */
  static std::map<State, std::vector<ControlCommand>> allowed_control_commands {
    { Idle, std::vector<ControlCommand> { ConfigureBeam, Terminate } },
    { BeamConfigured, std::vector<ControlCommand> { ConfigureScan, DeconfigureBeam } },
    { ScanConfigured, std::vector<ControlCommand> { StartScan, DeconfigureScan} },
    { Scanning, std::vector<ControlCommand> { StopScan } }
  };

  /**
   * @brief Mapping of the list of UpdateCommands that are valid for a specified state.
   * If the state does not exist in this mapping, then it has no valid UpdateCommands that
   * can be applied to it.
   *
   */
  static std::map<State, std::vector<UpdateCommand>> allowed_update_commands {
    { Unknown, { InitialisationComplete, InitialisationFailed } },
    { ConfiguringBeam, { ConfigureBeamComplete, ConfigureBeamFailed} },
    { ConfiguringScan, { ConfigureScanComplete, ConfigureScanFailed} },
    { StartingScan, { ScanStarted, ScanFailed} },
    { StoppingScan,  { ScanStopped, ScanFailed } },
    { DeconfiguringScan,  { DeconfigureScanComplete, DeconfigureScanFailed } },
    { DeconfiguringBeam, { DeconfigureBeamComplete, DeconfigureBeamFailed } }
  };

  /**
   * @brief Mapping of the ResetCommand that is valid for a specified state.
   * If the state does not exist in this mapping, then it has no valid ResetCommand that
   * can be applied to it.
   *
   */
  static std::map<State, ResetCommand> allowed_reset_commands {
    { ConfiguringBeamError, ResetConfiguringBeamError },
    { ConfiguringScanError, ResetConfiguringScanError },
    { ScanningError, ResetScanningError },
  };

  /**
   * @brief Mapping of a ControlCommand to the new State that command will result in.
   *
   */
  static std::map<ControlCommand, State> control_transitions {
    { ConfigureBeam, ConfiguringBeam },
    { ConfigureScan, ConfiguringScan },
    { StartScan, StartingScan },
    { StopScan, StoppingScan },
    { DeconfigureScan, DeconfiguringScan },
    { DeconfigureBeam, DeconfiguringBeam },
    { Terminate, Terminating }
  };

  /**
   * @brief Mapping of an UpdateCommand to the new State that command will result in.
   *
   */
  static std::map<UpdateCommand, State> update_transitions {
    { InitialisationComplete, Idle},
    { InitialisationFailed, InitialisationError },
    { ConfigureBeamComplete, BeamConfigured },
    { ConfigureBeamFailed, ConfiguringBeamError},
    { ConfigureScanComplete, ScanConfigured },
    { ConfigureScanFailed, ConfiguringScanError },
    { ScanStarted, Scanning },
    { ScanFailed, ScanningError },
    { ScanStopped, ScanConfigured },
    { DeconfigureScanComplete, BeamConfigured },
    { DeconfigureBeamComplete, Idle }
  };

  /**
   * @brief Mapping of a ResetCommand to the new State that command will result in.
   *
   */
  static std::map<ResetCommand, State> reset_transitions {
    { ResetConfiguringBeamError, Idle},
    { ResetConfiguringScanError, BeamConfigured },
    { ResetScanningError, ScanConfigured },
  };

  /**
   * @brief The state model that regulates the behaviour of the classes that inherit from it.
   * This model expects that commands will be executed on it from different threads
   *  - ControlCommands: these will be issued by an LmcService, and commence a state change from a steady state (e.g. Idle) to an intermediate state (e.g. ConfiguringBeam)
   *  - UpdateCommands: these will be issued by the child class, and conclude the transition from an intermediate state (e.g. ConfiguringBeam) to a steady state (e.g. BeamConfigured)
   *  - ResetCommands: these will be issued by the LmcService, and are used to reset the model from an error state, back to a steady state.
   *
   */
  class StateModel {

    public:

      /**
       * @brief Construct a new StateModel object, initialising the State to Unknown.
       *
       */
      StateModel(const std::string& entity);

      /**
       * @brief Destroy the StateModel object
       *
       */
      virtual ~StateModel() = default;

      /**
       * @brief Main thread of control for the implementing class, and uses UpdateCommands to transistion intermediate states to steady states.
       * Intended to be called in a separate std::thread from the LmcService that controls this state.
       *
       */
      void main();

      /**
       * @brief Issue the commands on the state model required to terminae the main method.
       *
       */
      void quit();

      /**
       * @brief Issues the ConfigureBeam command and waits for the BeamConfigured or ConfiguringBeamError state to be reached.
       *
       */
      virtual void configure_beam();

      /**
       * @brief Issues the ConfigureScan command and waits for the ScanConfigured or ConfiguringScanError state to be reached.
       *
       */
      virtual void configure_scan();

      /**
       * @brief Issues the StartScan command and waits for the Scanning or ScanningError state to be reached.
       *
       */
      virtual void start_scan();

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
       * @brief Issues the ResetConfiguringBeamError command and waits for the Idle state to be reached.
       *
       */
      virtual void reset_beam_configuration();

      /**
       * @brief Issues the ResetConfiguringScanError command and waits for the BeamConfigured state to tbe reached.
       *
       */
      virtual void reset_scan_configuration();

      /**
       * @brief Issues the ResetScanningError command and waits for the ScanConfigured state to tbe reached.
       *
       */
      virtual void reset_scanning();

      /**
       * @brief Initialisation callback to be implemented in the child class.
       * The method should wait for the Unknown state and transition the state model to the InitialisationComplete
       *
       */
      virtual void perform_initialise() = 0;

      /**
       * @brief Beam configuration callback that is called by \ref main to transition the state from ConfiguringBeam to BeamConfigured.
       *
       */
      virtual void perform_configure_beam() = 0;

     /**
       * @brief Scan configuration callback that is called by \ref main to transition the state from ConfiguringScan to ScanConfigured.
       *
       */
      virtual void perform_configure_scan() = 0;

      /**
       * @brief Scan callback that is called by \ref main to transition the state from StartingScan to Scanning.
       * This method is expected to block until the scan is complete.
       *
       */
      virtual void perform_scan() = 0;

      /**
       * @brief Deconfigure scan callback that is called by \ref main to transition the state from DeconfiguringScan to BeamConfigured.
       *
       */
      virtual void perform_deconfigure_scan() = 0;

      /**
       * @brief Deconfigure beam callback that is called by \ref main to transition the state from DeconfiguringBeam to Idle.
       *
       */
      virtual void perform_deconfigure_beam() = 0;

      /**
       * @brief Terminate callback that is called by \ref main to transition the state from Idle to Terminating.
       * 
       */
      virtual void perform_terminate() = 0;

      /**
       * @brief Set a pointer to the specified exception.
       * This mechanism is used by the StateModel acquire a reference to the most recent exception experience by the Receiver so that it can be retrived by the LmcService.
       *
       * @param exc exception to retain a pointer to.
       */
      void set_exception(const std::exception& exc);

      /**
       * @brief Return a pointer to the most recently received exception.
       *
       * @return std::exception_ptr
       */
      std::exception_ptr get_exception();

      /**
       * @brief Raise the exception pointed to by the last_exception attribute
       *
       */
      void raise_exception();

      /**
       * @brief Return the idle state
       *
       * @return true the model is idle
       * @return false the model is not idle
       */
      bool is_idle();

      /**
       * @brief Return resource assignment state
       *
       * @return true resources are assigned
       * @return false resources not assigned
       */
      bool is_beam_configured() const;

      /**
       * @brief Return the configuration state
       *
       * @return true the model is configured
       * @return false the model is not configured
       */
      bool is_scan_configured() const;

      /**
       * @brief Return the scanning state
       *
       * @return true the model is scanning or starting to scan
       * @return false the model is not scanning
       */
      bool is_scanning() const;

      /**
       * @brief Execute the control command on the state model.
       * Raises a runtime_error if the state model cannot accept the command.
       *
       * @param cmd control command to execute on the state model.
       */
      void execute(ControlCommand cmd);

      /**
       * @brief Execute the update command on the state model.
       * Raises a runtime_error if the state model cannot accept the command.
       *
       * @param cmd update command to execute on the state model.
       */
      void execute(UpdateCommand cmd);

      /**
       * @brief Execute the reset command on the state model.
       * Raises a runtime_error if the state model cannot accept the command.
       *
       * @param cmd reset command to execute on the state model.
       */
      void execute(ResetCommand cmd);

      /**
       * @brief Wait for the state model to transition to the required state.
       *
       * @param required state to wait for.
       */
      void wait_for(State required);

      /**
       * @brief Wait for the state model to transition to the expected state or the error state.
       * If the state model transitions to the error state, the exception raised by the Receiver
       * that caused the error will be raised here.
       *
       * @param expected expected state to wait for.
       * @param error error state to wait for.
       */
      void wait_for(State expected, State error);

      /**
       * @brief Wait for the state model to transition to any of the required states.
       *
       * @param required list of states to transition to
       * @return State the state model transitioned to
       */
      State wait_for(const std::vector<State>& required);

      /**
       * @brief Wait for the state model to transition to the required state with a timeout
       *
       * @param required required state to transition to
       * @param milliseconds timeout in milliseconds
       * @return true the state model achieved the transition to the required state before the timeout occurred
       * @return false the timeout occured and the state transition did not
       */
      bool wait_for(State required, unsigned milliseconds);

      /**
       * @brief Return the current state of the state model
       *
       * @return State current state of the state model
       */
      State get_state() { return state; }

      /**
       * @brief Return the name of the current state of the state model.
       *
       * @return std::string name of the current state of the state model
       */
      std::string get_name() { return get_name(get_state()); }

      /**
       * @brief Return the name of the specified control command.
       *
       * @param cmd control command whose name to return
       * @return std::string name of the command
       */
      std::string get_name(ControlCommand cmd) { return control_command_names[cmd]; }

      /**
       * @brief Return the name of the specified update command.
       *
       * @param cmd update command whose name to return
       * @return std::string name of the command
       */
      std::string get_name(UpdateCommand cmd) { return update_command_names[cmd]; }

      /**
       * @brief Return the name of the specified reset command.
       *
       * @param cmd reset command whose name to return
       * @return std::string name of the command
       */
      std::string get_name(ResetCommand cmd) { return reset_command_names[cmd]; }

      /**
       * @brief Query if the state model is in the required state
       *
       * @param required state to query
       * @return true the state model is in the required state
       * @return false the state model is not in the reuqired state
       */
      bool is(State required) { return state == required; }

      /**
       * @brief Return the name of the specified state
       *
       * @param s state whose name to return
       * @return std::string name of the specified state
       */
      static std::string get_name(State s) { return state_names[s]; }

      /**
       * @brief Check if the \ref is_beam_configured matches the expected value.
       *
       * @throw std::exception if \ref is_beam_configured does not match the expected value.
       * @param expected the expected value of \ref is_beam_configured.
       */
      void check_beam_configured(bool expected) const;

      /**
       * @brief Check if the \ref is_scan_configured matches the expected value.
       *
       * @throw std::exception if \ref is_scan_configured does not match the expected value.
       * @param expected the expected value of \ref is_scan_configured.
       */
      void check_scan_configured(bool expected) const;

      /**
       * @brief Check if the \ref is_scanning matches the expected value.
       *
       * @throw std::exception if \ref is_scanning does not match the expected value.
       * @param expected the expected value of \ref is_scanning.
       */
      void check_scanning(bool expected) const;

    private:

      /**
       * @brief Return whether the state model can currently accept the control cmd
       *
       * @param cmd control command to evaluate against the state model
       * @return true the state model can accept the control command
       * @return false the state model cannot accept the control command
       */
      bool allowed(ControlCommand cmd);

      /**
       * @brief Return whether the state model can currently accept the update cmd
       *
       * @param cmd update command to evaluate against the state model
       * @return true the state model can accept the update command
       * @return false the state model cannot accept the update command
       */
      bool allowed(UpdateCommand cmd);

      /**
       * @brief Return whether the state model can currently accept the reset cmd
       *
       * @param cmd reset command to evaluate against the state model
       * @return true the state model can accept the reset command
       * @return false the state model cannot accept the reset command
       */
      bool allowed(ResetCommand cmd);

      //! Current state of the state model
      State state{Unknown};

      //! Control access to the state attribute
      std::mutex mutex;

      //! Coordinates interactions with processes waiting on changes to the state attribute
      std::condition_variable cond;

      //! Reference to the most recently experienced exception
      std::exception_ptr last_exception{nullptr};

      //! Name of the agent using the state model
      std::string entity;
  };

} // common
} // pst
} // ska

#endif // SKA_PST_COMMON_StateModel_h