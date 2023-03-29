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

#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include "ska/pst/common/lmc/LmcService.h"
#include "ska/pst/common/lmc/LmcServiceHandler.h"
#include "ska/pst/common/statemodel/StateModel.h"
#include <spdlog/spdlog.h>

void ska::pst::common::LmcService::start() {
    SPDLOG_TRACE("ska::pst::common::LmcService::start()");
    SPDLOG_INFO("Starting gRPC LMC server '{}'", _service_name);
    grpc::internal::MutexLock lock(&_mu);
    if (_started) {
      return;
    }
    _started = true;

    _background_thread = std::make_unique<std::thread>([this](){
        serve();
    });

    while (!_server_ready) {
      _cond.Wait(&_mu);
    }
    _server_ready = false;
    SPDLOG_INFO("Started gRPC LMC server '{}' on port {}", _service_name, _port);
}

void ska::pst::common::LmcService::serve() {
    // this need to be on a background daemon thread.
    SPDLOG_TRACE("ska::pst::common::LmcService::serve()");
    std::string server_address("0.0.0.0:");
    server_address.append(std::to_string(_port));
    SPDLOG_TRACE("ska::pst::common::LmcService::serve setting up listen on port {}", _port);

    try
    {
        grpc::ServerBuilder builder;
        SPDLOG_TRACE("ska::pst::common::LmcService::serve adding TCP listener {} to service", server_address);

        SPDLOG_TRACE("ska::pst::common::LmcService::serve creating listener credentials");
        auto credentials = grpc::InsecureServerCredentials();
        SPDLOG_TRACE("ska::pst::common::LmcService::serve created credentials");

        SPDLOG_TRACE("ska::pst::common::LmcService::serve creating listener");
        builder.AddListeningPort(server_address, credentials, &_port);
        SPDLOG_TRACE("ska::pst::common::LmcService::serve listener created");

        SPDLOG_TRACE("ska::pst::common::LmcService::serve registering service.");
        builder.RegisterService(this);

        SPDLOG_TRACE("ska::pst::common::LmcService::serve starting server");
        server = builder.BuildAndStart();
        SPDLOG_TRACE("ska::pst::common::LmcService::serve listening on port {}", _port);

        grpc::internal::MutexLock lock(&_mu);
        _server_ready = true;
        _cond.Signal();
    } catch (std::exception& ex) {
        SPDLOG_ERROR("Error {} raised during startin up of gRPC service", ex.what());
        exit(1);
    }

}

void ska::pst::common::LmcService::stop() {
    SPDLOG_TRACE("ska::pst::common::LmcService::stop()");
    SPDLOG_INFO("Stopping gRPC LMC server '{}'", _service_name);
    grpc::internal::MutexLock lock(&_mu);
    if (!_started) {
      return;
    }
    server->Shutdown();
    _background_thread->join();

    _started = false;
}

void ska::pst::common::LmcService::set_state(
    ska::pst::lmc::ObsState state
)
{
    // lock the mutex used by monitor condition and notify that monitoring should stop
    {
        std::lock_guard<std::mutex> lk(_monitor_mutex);
        _state = state;
    }
    _monitor_condition.notify_all();
}

auto ska::pst::common::LmcService::connect(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::ConnectionRequest* request,
    ska::pst::lmc::ConnectionResponse* /*response*/
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::connect()");
    SPDLOG_INFO("gRPC LMC connection received from {}", request->client_id());

    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::configure_beam(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::ConfigureBeamRequest* request,
    ska::pst::lmc::ConfigureBeamResponse* /*response*/
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::configure_beam()");

    // check if handler has already have had beam configured
    if (handler->is_beam_configured()) {
        SPDLOG_WARN("Received configure beam request but beam configured already.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::CONFIGURED_FOR_BEAM_ALREADY);
        status.set_message(_service_name + " beam configured already. Beam configuation needs to be deconfigured before reconfiguring.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    if (_state != ska::pst::lmc::ObsState::EMPTY) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received configure beam request but not in EMPTY state. Currently in {} state.", curr_state_name);

        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in EMPTY state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    base_error_message = "Error in configuring beam";
    try {
        set_state(ska::pst::lmc::ObsState::RESOURCING);
        rethrow_application_manager_runtime_error("RuntimeError before configuring beam");
        handler->configure_beam(request->beam_configuration());
        set_state(ska::pst::lmc::ObsState::IDLE);

        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        std::string error_message = base_error_message + ": " + std::string(exc.what());
        SPDLOG_WARN(error_message);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message(error_message);
        set_state(ska::pst::lmc::ObsState::FAULT);
    }
}

auto ska::pst::common::LmcService::deconfigure_beam(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::DeconfigureBeamRequest* /*request*/,
    ska::pst::lmc::DeconfigureBeamResponse* /*response*/
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::deconfigure_beam()");

    // check if handler has already have had beam configured
    if (!handler->is_beam_configured()) {
        SPDLOG_WARN("Received request to deconfigure beam when no beam configured.");

        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::NOT_CONFIGURED_FOR_BEAM);
        status.set_message("No " + _service_name + " beam configured.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }


    base_error_message = "Error in deconfiguring beam";
    try {
        rethrow_application_manager_runtime_error("RuntimeError before deconfiguring beam");
        handler->deconfigure_beam();
        set_state(ska::pst::lmc::ObsState::EMPTY);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        std::string error_message = base_error_message + ": " + std::string(exc.what());
        SPDLOG_WARN(error_message);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message(error_message);
        set_state(ska::pst::lmc::ObsState::FAULT);

    }
}

auto ska::pst::common::LmcService::get_beam_configuration(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::GetBeamConfigurationRequest* /*request*/,
    ska::pst::lmc::GetBeamConfigurationResponse* response
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::get_beam_configuration()");
    if (!handler->is_beam_configured())
    {
        SPDLOG_WARN("Received request to get beam configuration when no beam configured.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::NOT_CONFIGURED_FOR_BEAM);
        status.set_message("No " + _service_name + " beam configured.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        auto *beam_configuration = response->mutable_beam_configuration();
        handler->get_beam_configuration(beam_configuration);

        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in getting beam configuration.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::configure_scan(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::ConfigureScanRequest* request,
    ska::pst::lmc::ConfigureScanResponse* /*response*/
) -> grpc::Status
{
    // check if handler has already been configured
    if (handler->is_scan_configured()) {
        SPDLOG_WARN("Received configure scan request but handler already has scan configured.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::CONFIGURED_FOR_SCAN_ALREADY);
        status.set_message(_service_name + " already configured for scan. Scan needs to be deconfigured before reconfiguring.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    // ensure in IDLE state
    if (_state != ska::pst::lmc::ObsState::IDLE) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received configure request but not in IDLE state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in IDLE state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    base_error_message = "Error in configuring scan";
    try {
        rethrow_application_manager_runtime_error("RuntimeError before configuring scan");
        handler->configure_scan(request->scan_configuration());
    } catch (std::exception& exc) {
        // handle exception
        std::string error_message = base_error_message + ": " + std::string(exc.what());
        SPDLOG_WARN(error_message);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message(error_message);
        set_state(ska::pst::lmc::ObsState::FAULT);
    }

    set_state(ska::pst::lmc::ObsState::READY);
    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::deconfigure_scan(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::DeconfigureScanRequest* /*request*/,
    ska::pst::lmc::DeconfigureScanResponse* /*response*/
) -> grpc::Status
{
    // ensure in READY state
    if (_state != ska::pst::lmc::ObsState::READY) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received deconfigure request but not in READY state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in READY state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    base_error_message = "Error in deconfiguring scan";
    try {
        rethrow_application_manager_runtime_error("RuntimeError before deconfiguring scan");
        handler->deconfigure_scan();
        set_state(ska::pst::lmc::ObsState::IDLE);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        std::string error_message = base_error_message + ": " + std::string(exc.what());
        SPDLOG_WARN(error_message);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message(error_message);
        set_state(ska::pst::lmc::ObsState::FAULT);
    }
}

auto ska::pst::common::LmcService::get_scan_configuration(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::GetScanConfigurationRequest* /*request*/,
    ska::pst::lmc::GetScanConfigurationResponse* response
) -> grpc::Status
{
    // ensure in READY state
    if (_state != ska::pst::lmc::ObsState::READY &&
        _state != ska::pst::lmc::ObsState::SCANNING
    ) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Get scan configuration request but not in configured state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in a configured state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        handler->get_scan_configuration(response->mutable_scan_configuration());
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in configuring beam.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }

    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::start_scan(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::StartScanRequest* request,
    ska::pst::lmc::StartScanResponse* /*response*/
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::scan()");
    if (_state == ska::pst::lmc::ObsState::SCANNING) {
        SPDLOG_WARN("Received scan request but already in SCANNING state.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::ALREADY_SCANNING);
        status.set_message(_service_name + " is already scanning.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }
    if (_state != ska::pst::lmc::ObsState::READY)
    {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received scan request but not in READY state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in READY state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    base_error_message = "Error in starting scan";
    try {
        rethrow_application_manager_runtime_error("RuntimeError before starting scan");
        handler->start_scan(*request);
        set_state(ska::pst::lmc::ObsState::SCANNING);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        std::string error_message = base_error_message + ": " + std::string(exc.what());
        SPDLOG_WARN(error_message);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message(error_message);
        set_state(ska::pst::lmc::ObsState::FAULT);
    }
}

auto ska::pst::common::LmcService::stop_scan(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::StopScanRequest* /*request*/,
    ska::pst::lmc::StopScanResponse* /*response*/
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::stop_scan()");
    if (_state != ska::pst::lmc::ObsState::SCANNING) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received stop scan request but not in SCANNING state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::NOT_SCANNING);

        std::ostringstream ss;
        ss << _service_name << " is not in SCANNING state. Currently in " << curr_state_name << " state.";
        status.set_message(ss.str());

        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    base_error_message = "Error in stopping scan";
    try {
        rethrow_application_manager_runtime_error("RuntimeError before stopping scan");
        handler->stop_scan();
        set_state(ska::pst::lmc::ObsState::READY);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        std::string e = exc.what();
        SPDLOG_WARN(base_error_message + ". Error: " + e);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in stopping scan request. Error: " + e);
        set_state(ska::pst::lmc::ObsState::FAULT);
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::get_state(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::GetStateRequest* /*request*/,
    ska::pst::lmc::GetStateResponse* response
) -> grpc::Status
{
    SPDLOG_TRACE("ska::pst::common::LmcService::get_state()");
    response->set_state(_state);
    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::monitor(
    grpc::ServerContext* context,
    const ska::pst::lmc::MonitorRequest* request,
    grpc::ServerWriter<ska::pst::lmc::MonitorResponse>* writer
) -> grpc::Status
{
    if (_state != ska::pst::lmc::ObsState::SCANNING) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received monitor but not in SCANNING state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::NOT_SCANNING);

        std::ostringstream ss;
        ss << _service_name << " is not in SCANNING state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    const auto &delay = std::chrono::milliseconds(request->polling_rate());

    while (true) {
        // use a condition variable and wait_for to allow interrupting the sleep, rather
        // than sleeping the thread and only checking condition after the sleep. Using
        // the predicate helps avoid spurious wakeups of blocked thread.
        //
        // See https://en.cppreference.com/w/cpp/thread/condition_variable/wait_for for
        // more details.
        std::unique_lock<std::mutex> lk(_monitor_mutex);
        if (_monitor_condition.wait_for(lk, delay, [this]{
            return _state != ska::pst::lmc::ObsState::SCANNING;
        }))
        {
            SPDLOG_INFO("No longer in SCANNING state. Exiting monitor");
            break;
        }
        SPDLOG_TRACE("Getting latest monitor data");

        ska::pst::lmc::MonitorResponse response;
        auto *monitor_data = response.mutable_monitor_data();
        handler->get_monitor_data(monitor_data);

        if (context->IsCancelled()) {
            SPDLOG_INFO("Monitoring context cancelled. Exiting monitor.");
            break;
        }
        if (!writer->Write(response)) {
            SPDLOG_WARN("Writing monitor response return false. Exiting monitor.");
            break;
        }
    }

    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::abort(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::AbortRequest* /*request*/,
    ska::pst::lmc::AbortResponse* /*response*/
) -> grpc::Status
{
    if (_state == ska::pst::lmc::ObsState::ABORTED)
    {
        SPDLOG_WARN("Received abort request but already in ABORTED state.");
        return grpc::Status::OK;
    }

    if (!(
        _state == ska::pst::lmc::ObsState::IDLE ||
        _state == ska::pst::lmc::ObsState::READY ||
        _state == ska::pst::lmc::ObsState::SCANNING
    )) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received abort request but not in an abortable state. Currently in {} state.",
            curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in an abortable state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        if (_state == ska::pst::lmc::ObsState::SCANNING)
        {
            handler->stop_scan();
        }
        set_state(ska::pst::lmc::ObsState::ABORTED);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in aborting.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::reset(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::ResetRequest* /*request*/,
    ska::pst::lmc::ResetResponse* /*response*/
) -> grpc::Status
{
    if (_state == ska::pst::lmc::ObsState::IDLE)
    {
        SPDLOG_WARN("Received reset request but in IDLE state. Ignoring request.");
        return grpc::Status::OK;
    }
    if (!(_state == ska::pst::lmc::ObsState::ABORTED or _state == ska::pst::lmc::ObsState::FAULT)) {
        // LMC is the source of truth, but we should have been moved to an ABORTED or FAULT state
        // before this could have been called.
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received reset request but not ABORTED or FAULT state. Currently in {} state.",
            curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in ABORTED or FAULT state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        if (handler->is_scan_configured())
        {
            handler->deconfigure_scan();
        }
        set_state(ska::pst::lmc::ObsState::IDLE);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in resetting.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::restart(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::RestartRequest* /*request*/,
    ska::pst::lmc::RestartResponse* /*response*/
) -> grpc::Status
{
    if (_state == ska::pst::lmc::ObsState::EMPTY)
    {
        SPDLOG_WARN("Received restart request but already in EMPTY state. Ignoring request.");
        return grpc::Status::OK;
    }
    if (!(_state == ska::pst::lmc::ObsState::ABORTED or _state == ska::pst::lmc::ObsState::FAULT)) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        SPDLOG_WARN("Received reset request but not ABORTED or FAULT state. Currently in {} state.",
            curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in ABORTED or FAULT state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        if (handler->is_scan_configured())
        {
            handler->deconfigure_scan();
        }
        if (handler->is_beam_configured())
        {
            handler->deconfigure_beam();
        }
        set_state(ska::pst::lmc::ObsState::EMPTY);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in resetting.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::go_to_fault(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::GoToFaultRequest* /*request*/,
    ska::pst::lmc::GoToFaultResponse* /*response*/
) -> grpc::Status
{
    try {
        // Try to stop scanning
        if (handler->is_scanning())
        {
            handler->stop_scan();
        }
    } catch (std::exception& ex) {
        SPDLOG_WARN("{} gRPC service tried to stop scanning but exception {} occurred.", _service_name, ex.what());
    }
    set_state(ska::pst::lmc::ObsState::FAULT);
    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::get_env(
    grpc::ServerContext* /*context*/,
    const ska::pst::lmc::GetEnvironmentRequest* /*request*/,
    ska::pst::lmc::GetEnvironmentResponse* response
) -> grpc::Status
{
    handler->get_env(response);
    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::rethrow_application_manager_runtime_error(
    const std::string& _base_error_message
) -> void
{
    if (handler->get_application_manager_state() == ska::pst::common::RuntimeError)
    {
        if (handler->get_application_manager_exception())
        {
            base_error_message = _base_error_message;
            std::rethrow_exception(handler->get_application_manager_exception());
        }
    }
}