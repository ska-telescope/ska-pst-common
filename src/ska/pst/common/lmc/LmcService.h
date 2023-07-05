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

#ifndef __SKA_PST_COMMON_LmcService_h
#define __SKA_PST_COMMON_LmcService_h

#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <grpc++/grpc++.h>

#include "absl/memory/memory.h"

#include "ska/pst/common/lmc/LmcServiceHandler.h"
#include "ska/pst/lmc/ska_pst_lmc.grpc.pb.h"

namespace ska::pst::common {

    /**
     * @brief Class to handle the local monitoring and control of PST Services.
     *
     * This is a gRPC service implementation that can be used by remote clients,
     * such as the SMRB.LMC and RECV.LMC, to manage a PST signal processing
     * application. Applications are expected to not extend this class, or
     * implement their own version of the ska::pst::lmc::PstLmcService::Service
     * interface. This will allow for a common interface that the PST.LMC can
     * use and expect the same behaviour no matter what.
     *
     * Services are expected to provide an implementation of the
     * ska::pst::common::LmcServiceHandler which provides a bridge pattern
     * between instances of this service and what is specific to the
     * signal processing application.
     */
    class LmcService final : public ska::pst::lmc::PstLmcService::Service {
        private:
            /**
             * @brief The name of the service.
             *
             * This is used in reporting error messages back to the client.
             */
            std::string _service_name;

            /**
             * @brief The TCP port used by the gRPC server.
             *
             */
            int _port;

            /**
             * @brief The instance of the gRPC server for this service.
             *
             * The instance of this service will be registered with the server
             * when the start() command is called.
             */
            std::unique_ptr<grpc::Server> server{nullptr};

            /**
             * @brief A pointer to the handler that used by this service.
             *
             * Commands that this service responds too will be proxied to
             * the handler which will provide the specific implementation
             * for the command, such as handling the resource allocation
             * or configuring a scan.
             */
            std::shared_ptr<ska::pst::common::LmcServiceHandler> handler;

            /**
             * @brief The background thread used for the running of the service.
             *
             * To make sure that the gRPC Server is run in the background, a thread
             * is used to start it. This field is a pointer to that thread.
             */
            std::unique_ptr<std::thread> _background_thread{nullptr};

            /**
             * @brief A gRPC specific mutex used in th starting/stopping of the gRPC server.
             *
             */
            grpc::internal::Mutex _mu;

            /**
             * @brief A gRPC specific conditiona variable used in th starting/stopping of the gRPC server.
             *
             */
            grpc::internal::CondVar _cond;

            /**
             * @brief A guarded variable used to notify when the gRPC server is ready to serve requests.
             *
             */
            bool _server_ready ABSL_GUARDED_BY(_mu) = false;

            /**
             * @brief A guarded variable used as a simple state machine of the service.
             *
             * If false, then the service hasn't been started.  If true the service has been started and
             * the start() method can't be called again.
             */
            bool _started ABSL_GUARDED_BY(_mu) = false;

            /**
             * @brief The method called in the background thread for running the gRPC server.
             *
             */
            void serve();

            /**
             * @brief Rethrows the error of an ApplicationManager instance if its state
             * is in a RuntimeError
             *
             */
            void rethrow_application_manager_runtime_error(const std::string& _base_error_message);

            /**
             * @brief The SKA Observation State of the LMC service.
             *
             * Used to keep track of the state of this service. This is used to guard against having
             * the methods called in the wrong order.
             */
            ska::pst::lmc::ObsState _state{ska::pst::lmc::ObsState::EMPTY};

            /**
             * @brief A utitlity method to set the current state.
             *
             * This will use a mutex and a condition variable to update the _state and allow for
             * background threads, like monitoring, to be notified of a state change.
             */
            void set_state(ska::pst::lmc::ObsState);

            /**
             * @brief A mutex used by guard the read/write of the state value.
             *
             * This mutex is used to guard updating of the state variable to the monitoring
             * thread can use a condition variable to be awoken when the state has changed.
             */
            std::mutex _monitor_mutex;

            /**
             * @brief A threading primative that use by the monitoring thread to know when
             * the state has change.
             *
             */
            std::condition_variable _monitor_condition;

            /**
             * @brief Common variable used for appending context to error messages.
             *
             */
            std::string base_error_message = "";

        public:
            /**
             * @brief Constructor for the LMC service.
             *
             * @param service_name the name of this service, used within error reporting back to client.
             * @param handler a handler instance that brigdes this service to the application.
             * @param port the TCP port that this service is exposed on.
             */
            LmcService(std::string service_name, std::shared_ptr<ska::pst::common::LmcServiceHandler> handler, int port) : _service_name(service_name), handler(std::move(handler)), _port(port) {}

            /**
             * @brief Default deconstructor of service.
             */
            virtual ~LmcService() = default;

            /**
             * @brief Return the name of the LMC service
             *
             * @return std::string the name of the LMC service set in the constructor
             */
            std::string service_name() { return _service_name; }

            /**
             * @brief Start the gRPC process in the background.
             *
             */
            void start();

            /**
             * @brief Stops the gRPC service is it is running.
             *
             */
            void stop();

            /**
             * @brief Check if the service is running.
             *
             */
            bool is_running() { return _started; }

            /**
             * @brief Retrieve port server is running on.
             *
             */
            int port() { return _port; }

            /**
             * @brief Implements the connect method for the LMC gRPC service.
             *
             * This method is used by clients to see if they can connect to the service,
             * and is otherwise a no-op method.
             */
            grpc::Status connect(grpc::ServerContext* context, const ska::pst::lmc::ConnectionRequest* request, ska::pst::lmc::ConnectionResponse* response) override;

            /**
             * @brief Implements the configure beam functionality of the LMC gRPC service.
             *
             * The implementation of this does not check that the method is directed at the right service, this is
             * delegated to the @see LmcServiceHandler implementation which should assert the method is for the right service.
             *
             * If the service has already been configured for beam then this will set the status to being FAILED_PRECONDITION and provide
             * details within a serialised version of a ska::pst::lmc::Status message.
             */
            grpc::Status configure_beam(grpc::ServerContext* context, const ska::pst::lmc::ConfigureBeamRequest* request, ska::pst::lmc::ConfigureBeamResponse* response) override;

            /**
             * @brief Implements releasing the beam resources of the LMC gRPC service.
             *
             * This method will ensure that the service is no long using beam resources, such as
             * being connected to ring buffers. It is only valid to call this method if the state of the service
             * is in an IDLE state, meaning it can't be scanning, configured for a scan or in an
             * aborted state. Failure to meet the required precondition will result in a gRPC status
             * of FAILED_PRECONDITION and provide details within a serialised version of a
             * ska:pst::lmc::Status message.
             */
            grpc::Status deconfigure_beam(grpc::ServerContext* context, const ska::pst::lmc::DeconfigureBeamRequest* request, ska::pst::lmc::DeconfigureBeamResponse* response) override;

            /**
             * @brief Implements getting the current beam configuration of the LMC gRPC service.
             *
             * Returns the current beam configuration for the service. If the service is not configured for beam
             * then this will return a status with FAILED_PRECONDITION and provide
             * details within the serialised version of a ska::pst::lmc::Status message.
             */
            grpc::Status get_beam_configuration(grpc::ServerContext* context, const ska::pst::lmc::GetBeamConfigurationRequest* request, ska::pst::lmc::GetBeamConfigurationResponse* response) override;

            /**
             * @brief Implements configuring the service for in preparation for a scan.
             *
             * This will configure the service in ready for a scan. For COMMON this is effectively a
             * no-op method, though it does assert that the service is in IDLE state (i.e. has been
             * configured for a beam) and afterwards will put the state into READY.
             */
            grpc::Status configure_scan(grpc::ServerContext* context, const ska::pst::lmc::ConfigureScanRequest* request, ska::pst::lmc::ConfigureScanResponse* response) override;

            /**
             * @brief Implements deconfiguring the service so that its not ready for scanning.
             *
             * This will deconfigure the service in ready for a scan. For COMMON this is effectively a
             * no-op method, though it asserts the service is in a READY state (i.e. is ready for
             * scanning but is not actually scanning). This method would put the service back into
             * and IDLE state (i.e. is still configured for a beam but is not ready for scanning).
             */
            grpc::Status deconfigure_scan(grpc::ServerContext* context, const ska::pst::lmc::DeconfigureScanRequest* request, ska::pst::lmc::DeconfigureScanResponse* response) override;

            /**
             * @brief Implements getting the current scan configuration.
             *
             * This is a no-op operation for COMMON. It is only valid to call this when the state is
             * either READY or SCANNING.
             */
            grpc::Status get_scan_configuration(grpc::ServerContext* context, const ska::pst::lmc::GetScanConfigurationRequest* request, ska::pst::lmc::GetScanConfigurationResponse* response) override;

            /**
             * @brief Implements start scanning of the gRPC service.
             *
             * This will put the service into the SCANNING state. This is only valid if the current
             * state of the service is a READY (i.e. it is configured for a beam and has been configured
             * for a scan). If the service is already SCANNING or not in a READY state will result in
             * a gRPC status of FAILED_PRECONDITION and provide details within a serialised version of a
             * ska:pst::lmc::Status message.
             */
            grpc::Status start_scan(grpc::ServerContext* context, const ska::pst::lmc::StartScanRequest* request, ska::pst::lmc::StartScanResponse* response) override;

            /**
             * @brief Implements end scanning of the LMC gRPC service.
             *
             * This will stop a currently running scan and notify any background monitoring to stop.
             * If this completes successfully the state of the service is put back into READY to be
             * able to perform another scan.
             */
            grpc::Status stop_scan(grpc::ServerContext* context, const ska::pst::lmc::StopScanRequest* request, ska::pst::lmc::StopScanResponse* response) override;

            /**
             * @brief Implements get the current observation state of the LMC gRPC serivce.
             *
             */
            grpc::Status get_state(grpc::ServerContext* context, const ska::pst::lmc::GetStateRequest* request, ska::pst::lmc::GetStateResponse* response) override;

            /**
             * @brief Implements the monitoring of data for the COMMON service.
             *
             * Statitics of the service during the scanning, such as the read/write statistics for
             * ring buffers (SMRB) or the amount of data received (RECV).
             *
             * This can only be called if service is in a scanning state. If the client drops
             * the request then this method will shut down gracefully.
             */
            grpc::Status monitor(grpc::ServerContext* context, const ska::pst::lmc::MonitorRequest* request, grpc::ServerWriter< ska::pst::lmc::MonitorResponse>* writer) override;

            /**
             * @brief Implements the aborting of processes for the COMMON service.
             *
             * For COMMON the only process that needs to be aborted monitoring, as all the other
             * commands are short lived commands.  This implementation will update the state
             * of this service to be ABORTED which means other commands can't do anything until
             * as reset and or restart is sent.
             */
            grpc::Status abort(grpc::ServerContext* context, const ska::pst::lmc::AbortRequest* request, ska::pst::lmc::AbortResponse* response) override;

            /**
             * @brief Implements the resetting of the service.
             *
             * This method will move the COMMON service from an aborted/fault state back to IDLE
             * That it is has been put into a configured for a beam state, but is not configured for a scan.
             */
            grpc::Status reset(grpc::ServerContext* context, const ska::pst::lmc::ResetRequest* request, ska::pst::lmc::ResetResponse* response) override;

            /**
             * @brief Implements the restarting of the service.
             *
             * This method will move the COMMON service from an aborted/fault state back to EMPTY.
             * This will make sure that the service is deconfigured completely, including releasing ring buffers.
             * If not configured for a beam then this will just move to EMPTY.
             */
            grpc::Status restart(grpc::ServerContext* context, const ska::pst::lmc::RestartRequest* request, ska::pst::lmc::RestartResponse* response) override;

            /**
             * @brief Implements putting the service in the FAULT state.
             *
             * Used by a client to set the service into a FAULT state. This
             * can be due to another part of the logical BEAM being faulty
             * and a call to this method from a client it advising the service
             * to move to the FAULT state.
             *
             * By moving to the FAULT state the service can be reset or
             * restarted.
             */
            grpc::Status go_to_fault(grpc::ServerContext* context, const ska::pst::lmc::GoToFaultRequest* request, ska::pst::lmc::GoToFaultResponse* response) override;

            /**
             * @brief Implements getting the environment values from the service.
             *
             * This is used by clients to get environment specific to the service, such
             * as the IP addresses and ports that the service exposes.
             */
            grpc::Status get_env(grpc::ServerContext* context, const ska::pst::lmc::GetEnvironmentRequest* request, ska::pst::lmc::GetEnvironmentResponse* response) override;

            /**
             * @brief Implements setting the log level of the service.
             */
            grpc::Status set_log_level(grpc::ServerContext* context, const ska::pst::lmc::SetLogLevelRequest* request, ska::pst::lmc::SetLogLevelResponse* response) override;

            /**
             * @brief Implements getting the log level of the service.
             */
            grpc::Status get_log_level(grpc::ServerContext* context, const ska::pst::lmc::GetLogLevelRequest* request, ska::pst::lmc::GetLogLevelResponse* response) override;
    };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_LmcService_h