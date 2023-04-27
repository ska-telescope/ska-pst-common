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

#ifndef __SKA_PST_COMMON_LmcServiceHandler_h
#define __SKA_PST_COMMON_LmcServiceHandler_h

#include <string>

#include "ska/pst/lmc/ska_pst_lmc.pb.h"
#include "ska/pst/common/statemodel/StateModel.h"

namespace ska::pst::common {

    /**
     * @brief A purely abstract class that is used as a bridge between gRPC service and PST applications.
     *
     * The @see ska::pst::common::LmcService class uses a pointer to an instance of this abstract class
     * to act as a bridge between the common expected Local Monitoring and Control functionality and
     * the specific implementation for a PST signal processing application.
     *
     * Implementations of this can have their own state model, but the @see ska::pst::common::LmcService
     * will enforce the SKA Obs State model to ensure that the call to the handler should be in a
     * valid state.
     */
    class LmcServiceHandler {
        public:
            // beam resourcing methods
            /**
             * @brief Handle the beam configuration for the service.
             *
             * Implementations of this method should enforce that the correct
             * sub-field in the ska::pst::lmc::BeamConfiguration message is set, (e.g. that
             * for SMRB the smrb field is set, and similarly for RECV the receive field is set.)
             *
             * The implementation should check its own state model but the service calling this
             * method has asserted that the service has not been configured for a beam and the service is in
             * the EMPTY ObsState.
             *
             * @param configuration the configuration for the beam. This message has oneof field and should
             *      match that of the service.
             * @throw std::exception if there is a validation issue or problem with the beam configuration of the service.
             */
            virtual void configure_beam(const ska::pst::lmc::BeamConfiguration &configuration) = 0;

            /**
             * @brief Handle deconfiguring the service from a beam.
             *
             * Implementations of this method should release the beam resources for the service, including
             * disconnecting from any ring buffers.
             *
             * The implementation should check its own state model but the service calling this method
             * has asserted that the service is configured for a beam and in an IDLE ObsState.
             *
             * @throw std::exception if there is a validation issue or problem with beam deconfiguration of the service.
             */
            virtual void deconfigure_beam() = 0;

            /**
             * @brief Handle getting the current beam configuration for the service.
             *
             * Implementations of this method should return the beam configuration in the sub-field message
             * that relates to the service implementation. That is, for SMRB the implementation should
             * set the smrb field, likewise for RECV setting the receive field, etc.
             *
             * The implementation should check its own state model that beam configuration is set, but the service
             * calling this method has checked that there is beam configuration and is not in EMPTY ObState.
             *
             * @param configuration Pointer to the protobuf message to return. Implementations should get
             *      mutable references to the sub-field they are responding to and update that message.
             * @throw std::exception if there is a validation issue or problem with getting beam configuration.
             */
            virtual void get_beam_configuration(ska::pst::lmc::BeamConfiguration* configuration) = 0;

            /**
             * @brief Check if this service is configured for a beam.
             *
             * Implementations of this method are required to return true if the service has been configured
             * for a beam, else false. Also implementations of this should not throw an exception.
             */
            virtual bool is_beam_configured() const noexcept = 0;

            // scan configuration methods
            /**
             * @brief Handle configuring the service for a scan.
             *
             * Implementations of this method should enforce check and enforce that the correct
             * sub-field in the ska::pst::lmc::ScanConfiguration message is set, (e.g. that
             * for SMRB the smrb field is set, and similarly for RECV the receive field is set.)
             *
             * The implementation should check its own state model but the service calling this
             * methods has asserted that no scan is currently configured and the server is in
             * the IDLE ObsState.
             *
             * @param configuration the scan configuration to use. This message has oneof field and should
             *      match that of the service.
             * @throw std::exception if there is a validation issue or problem with configuring a scan.
             */
            virtual void configure_scan(const ska::pst::lmc::ScanConfiguration &configuration) = 0;

            /**
             * @brief Handle deconfiguring service for a scan.
             *
             * Implementations of this method should reset any scan configuration parameters that it
             * currently has set. It should not deconfigure any beam resources that have been set from a call
             * to configure_beam.
             *
             * The implementation should check its own state model but the service calling this method
             * has been configured for a scan and in a READY ObsState.
             *
             * @throw std::exception if there is a validation issue or problem with deconfiguring the scan.
             */
            virtual void deconfigure_scan() = 0;

            /**
             * @brief Handle getting the current scan configuration for the service.
             *
             * Implementations of this method should return the current scan configuration in the
             * sub-field message that relates to the service implementation. That is, for SMRB the
             * implementation should set the smrb field, likewise for RECV setting the receive field, etc.
             *
             * The implementation should check its own state model that there is a scan configuration set, but the service
             * calling this method that has been configured for a scan and in either READY or SCANNING ObState.
             *
             * @param configuration Pointer to the protobuf message to return. Implementations should get
             *      mutable references to the sub-field they are responding to and update that message.
             * @throw std::exception if there is a validation issue or problem with getting the scan configuration.
             */
            virtual void get_scan_configuration(ska::pst::lmc::ScanConfiguration *configuration) = 0;

            /**
             * @brief Check if the service has been configured for a scan.
             *
             * Implementations of this method are required to return true if has been configured for a scan,
             * else false. Also implementations of this should not throw an exception.
             */
            virtual bool is_scan_configured() const noexcept = 0;

            // scan methods
            /**
             * @brief Handle initiating a scan.
             *
             * Upon a successful return of this method the service should be processing scan data
             * (e.g. for RECV it should be processing data from UDP port, and DSP should process the
             * data in the data block.)
             *
             * The implementation should check its own state model but the service calling this
             * methods has asserted that no scan is happening and the server is in a READY ObsState (i.e.
             * that a scan has been configured but not running).
             *
             * @param request the request to use for the scan. Note at the moment the ScanRequest method
             *      is empty but this may change in the future such as the time offset to when to start.
             * @throw std::exception if there is a validation issue or problem starting a scan.
             */
            virtual void start_scan(const ska::pst::lmc::StartScanRequest &request) = 0;

            /**
             * @brief Handle ending a scan.
             *
             * Implementations of this should stop processing signal data and return to a state which
             * would allow a new scan if requested.
             *
             * The implementation should check its own state model but the service calling this
             * methods has asserted that service is scanning and is in a SCANNING ObsState.
             *
             * Implementations of this should try not throw an exception but gracefully stop scanning.
             * If this can't happen then it would actually be impossible for the service to abort
             * scanning as a call to the LMC service of abort will try to cancel a scan if it is running.
             *
             * @throw std::exception if there is a validation issue or problem stopping a scan.
             */
            virtual void stop_scan() = 0;

            /**
             * @brief Check if the service is currenting performing a scan.
             *
             * Implementations of this method are required to return true a scan is happening,
             * else false. Also implementations of this should not throw an exception.
             */
            virtual bool is_scanning() const noexcept = 0;

            // monitoring
            /**
             * @brief Handle getting the monitoring data for the service.
             *
             * Implementations of this method should return the current monitoring data. While the
             * LMC service is doing this in a background thread the implementation of the service
             * may also have its more real time background processing of monitoring (i.e. RECV
             * can have a background thread to get data and this method could get a snapshot of that
             * data). Implemenations should update the sub-field message that relates to the service
             * implementation.
             *
             * The implementation should check its own state model that service is performning as scan,
             * but the service calling this method is scanning and is in a SCANNING ObsState.
             *
             * @param data Pointer to the protobuf message to return. Implementations should get
             *      mutable references to the sub-field they are responding to and update that message.
             * @throw std::exception if there is a validation issue or problem during monitoring.
             */
            virtual void get_monitor_data(ska::pst::lmc::MonitorData *data) = 0;

            // get environment
            /**
             * @brief Return environment variables back to the client.
             *
             * The default implementation of this is a no-op. However, implementations that override this
             * method should document clearly what values they are returning, including the key, a
             * description of the value, and the value type.  This will allow the clients of gRPC service
             * to know what to expect.
             *
             * Implementations must not throw exceptions calling this method, they should just return
             * a empty response.
             *
             * @param data Pointer to a protobuf message message that includes the a map to populate.
             */
             virtual void get_env(ska::pst::lmc::GetEnvironmentResponse *data) noexcept {}

            /**
             * @brief Implements the resetting of the ApplicationManager.
             *
             * This method will set the state of an ApplicationManager into IDLE
             */
            virtual void reset() = 0;

            /**
             * @brief Return State of an ApplicationManager
             *
             */
             virtual ska::pst::common::State get_application_manager_state() = 0;
             /**
              * @brief Get the exception object
              *
              * @return std::exception_ptr
              */
             virtual std::exception_ptr get_application_manager_exception() = 0;

             /**
              * @brief Put application into a runtime error state.
              *
              * @param exception the exception to store on the application manager.
              */
             virtual void go_to_runtime_error(std::exception exception) = 0;
    };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_LmcServiceHandler_h

