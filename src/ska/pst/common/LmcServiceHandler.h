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

namespace ska {
namespace pst {
namespace common {

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
             * @brief Handle assigning of resources for the service.
             *
             * Implementations of this method should enforce check and enforce that the correct
             * sub-field in the ska::pst::lmc::ResourceConfiguration message is set, (e.g. that
             * for SMRB the smrb field is set, and similarly for RECV the receive field is set.)
             *
             * The implementation should check its own state model but the service calling this
             * methods has asserted that no resources have been assigned and the server is in
             * the EMPTY ObsState.
             *
             * @param resources the resources to assign. This message has oneof field and should
             *      match that of the service.
             * @throw std::exception if there is a validation issue or problem with assigning resources.
             */
            virtual void assign_resources(const ska::pst::lmc::ResourceConfiguration &resources) = 0;

            /**
             * @brief Handle releasing of assigned resources.
             *
             * Implementations of this method should release the assigned resources for the service.
             * The implementation should check its own state model but the service calling this method
             * has asserted that resources are assigned and in an IDLE ObsState.
             *
             * @throw std::exception if there is a validation issue or problem with assigning resources.
             */
            virtual void release_resources() = 0;

            /**
             * @brief Handle getting the currently assigned resources for the service.
             *
             * Implementations of this method should return the resources in the sub-field message
             * that relates to the service implementation. That is, for SMRB the implementation should
             * set the smrb field, likewise for RECV setting the receive field, etc.
             *
             * The implementation should check its own state model that resources are set, but the service
             * calling this method that resources have been assigned and not in EMPTY ObState.
             *
             * @param resources Pointer to the protobuf message to return. Implementations should get
             *      mutable references to the sub-field they are responding to and update that message.
             * @throw std::exception if there is a validation issue or problem with assigning resources.
             */
            virtual void get_assigned_resources(ska::pst::lmc::ResourceConfiguration* resources) = 0;

            /**
             * @brief Check if resources are assigned to this service.
             *
             * Implementations of this method are required to return true if resources have been
             * assigned, else false. Also implementations of this should not throw an exception.
             */
            virtual bool are_resources_assigned() const noexcept = 0;

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
             * @param resources the scan configuration to use. This message has oneof field and should
             *      match that of the service.
             * @throw std::exception if there is a validation issue or problem with configuring a scan.
             */
            virtual void configure(const ska::pst::lmc::ScanConfiguration &configuration) = 0;

            /**
             * @brief Handle deconfiguring service for a scan.
             *
             * Implementations of this method should reset any scan configuration parameters that it
             * currently has set. It should not release any resources that have been set from a call
             * to assign_resources.
             *
             * The implementation should check its own state model but the service calling this method
             * has been configured for a scan and in a READY ObsState.
             *
             * @throw std::exception if there is a validation issue or problem with assigning resources.
             */
            virtual void deconfigure() = 0;

            /**
             * @brief Handle getting the current scan configuration for the service.
             *
             * Implementations of this method should return the current scan configuration in the
             * sub-field message that relates to the service implementation. That is, for SMRB the
             * implementation should set the smrb field, likewise for RECV setting the receive field, etc.
             *
             * The implementation should check its own state model that resources are set, but the service
             * calling this method that has been configured for a scan and in either READY or SCANNING ObState.
             *
             * @param configuration Pointer to the protobuf message to return. Implementations should get
             *      mutable references to the sub-field they are responding to and update that message.
             * @throw std::exception if there is a validation issue or problem with assigning resources.
             */
            virtual void get_scan_configuration(ska::pst::lmc::ScanConfiguration *configuration) = 0;

            /**
             * @brief Check if the service has been configured for a scan.
             *
             * Implementations of this method are required to return true if has been configured for a scan,
             * else false. Also implementations of this should not throw an exception.
             */
            virtual bool is_configured() const noexcept = 0;

            // scan method
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
            virtual void scan(const ska::pst::lmc::ScanRequest &request) = 0;

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
            virtual void end_scan() = 0;

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
             * @throw std::exception if there is a validation issue or problem with assigning resources.
             */
            virtual void get_monitor_data(ska::pst::lmc::MonitorData *data) = 0;
    };

} // common
} // pst
} // ska

#endif // __SKA_PST_COMMON_LmcServiceHandler_h

