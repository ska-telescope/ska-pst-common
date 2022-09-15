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
#include <spdlog/spdlog.h>

void ska::pst::common::LmcService::start() {
    spdlog::trace("ska::pst::common::LmcService::start()");
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
    spdlog::info("gRPC LMC server started on port {}", _port);
}

void ska::pst::common::LmcService::serve() {
    // this need to be on a background daemon thread.
    spdlog::trace("ska::pst::common::LmcService::serve()");
    std::string server_address("0.0.0.0:");
    server_address.append(std::to_string(_port));
    spdlog::trace("ska::pst::common::LmcService::serve setting up listen on port {}", _port);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials(), &_port);
    builder.RegisterService(this);

    spdlog::trace("ska::pst::common::LmcService::serve starting server");
    server = builder.BuildAndStart();
    spdlog::trace("ska::pst::common::LmcService::serve listening on port {}", _port);

    grpc::internal::MutexLock lock(&_mu);
    _server_ready = true;
    _cond.Signal();
}

void ska::pst::common::LmcService::stop() {
    spdlog::trace("ska::pst::common::LmcService::stop()");
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
    grpc::ServerContext* context,
    const ska::pst::lmc::ConnectionRequest* request,
    ska::pst::lmc::ConnectionResponse* /*response*/
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::connect()");
    spdlog::info("gRPC LMC connection received from {}", request->client_id());
    
    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::assign_resources(
    grpc::ServerContext* context,
    const ska::pst::lmc::AssignResourcesRequest* request,
    ska::pst::lmc::AssignResourcesResponse* /*response*/
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::assign_resources()");

    // check if handler has already have had resources assigned
    if (handler->are_resources_assigned()) {
        spdlog::warn("Received assign resources request but resources already assigned.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::RESOURCES_ALREADY_ASSIGNED);
        status.set_message(_service_name + " resources already assigned. Resources need to be released before reassigning.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    if (_state != ska::pst::lmc::ObsState::EMPTY) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received assign resources request but not in EMPTY state. Currently in {} state.", curr_state_name);

        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in EMPTY state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        set_state(ska::pst::lmc::ObsState::RESOURCING);
        handler->assign_resources(request->resource_configuration());
        set_state(ska::pst::lmc::ObsState::IDLE);

        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in assigning resources.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::release_resources(
    grpc::ServerContext* context,
    const ska::pst::lmc::ReleaseResourcesRequest* /*request*/,
    ska::pst::lmc::ReleaseResourcesResponse* /*response*/
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::release_resources()");

    // check if data manager has already have had resources assigned
    if (!handler->are_resources_assigned()) {
        spdlog::warn("Received request to release resources when no resources are assigned.");

        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::RESOURCES_NOT_ASSIGNED);
        status.set_message("No " + _service_name + " resources assigned.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        handler->release_resources();
        set_state(ska::pst::lmc::ObsState::EMPTY);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in releasing resources.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::get_assigned_resources(
    grpc::ServerContext* context,
    const ska::pst::lmc::GetAssignedResourcesRequest* /*request*/,
    ska::pst::lmc::GetAssignedResourcesResponse* response
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::get_assigned_resources()");
    if (!handler->are_resources_assigned())
    {
        spdlog::warn("Received request to get assigned resources when no resources are assigned.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::RESOURCES_NOT_ASSIGNED);
        status.set_message("No " + _service_name + " resources assigned.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        auto *resource_configuration = response->mutable_resource_configuration();
        handler->get_assigned_resources(resource_configuration);

        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in getting assigned resources.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::configure(
    grpc::ServerContext* context,
    const ska::pst::lmc::ConfigureRequest* request,
    ska::pst::lmc::ConfigureResponse* /*response*/
) -> grpc::Status
{
    // check if handler has already been configured
    if (handler->is_configured()) {
        spdlog::warn("Received configure scan request but handler already has scan configured.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::SCAN_CONFIGURED_ALREADY);
        status.set_message(_service_name + " already configured for scan. Scan needs to be deconfigured before reconfiguring.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    // ensure in IDLE state
    if (_state != ska::pst::lmc::ObsState::IDLE) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received configure request but not in IDLE state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in IDLE state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        handler->configure(request->scan_configuration());
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in assigning resources.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }

    set_state(ska::pst::lmc::ObsState::READY);
    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::deconfigure(
    grpc::ServerContext* context,
    const ska::pst::lmc::DeconfigureRequest* /*request*/,
    ska::pst::lmc::DeconfigureResponse* /*response*/
) -> grpc::Status
{
    // ensure in READY state
    if (_state != ska::pst::lmc::ObsState::READY) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received deconfigure request but not in READY state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in READY state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        handler->deconfigure();
        set_state(ska::pst::lmc::ObsState::IDLE);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in assigning resources.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::get_scan_configuration(
    grpc::ServerContext* context,
    const ska::pst::lmc::GetScanConfigurationRequest* /*request*/,
    ska::pst::lmc::GetScanConfigurationResponse* response
) -> grpc::Status
{
    // ensure in READY state
    if (_state != ska::pst::lmc::ObsState::READY &&
        _state != ska::pst::lmc::ObsState::SCANNING
    ) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Get scan configuration request but not in configured state. Currently in {} state.", curr_state_name);
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
        status.set_message("Error in assigning resources.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }

    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::scan(
    grpc::ServerContext* context,
    const ska::pst::lmc::ScanRequest* request,
    ska::pst::lmc::ScanResponse* /*response*/
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::scan()");
    if (_state == ska::pst::lmc::ObsState::SCANNING) {
        spdlog::warn("Received scan request but already in SCANNING state.");
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::ALREADY_SCANNING);
        status.set_message(_service_name + " is already scanning.");
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }
    if (_state != ska::pst::lmc::ObsState::READY)
    {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received scan request but not in READY state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in READY state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        handler->scan(*request);
        set_state(ska::pst::lmc::ObsState::SCANNING);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in starting scan request.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::end_scan(
    grpc::ServerContext* context,
    const ska::pst::lmc::EndScanRequest* /*request*/,
    ska::pst::lmc::EndScanResponse* /*response*/
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::end_scan()");
    if (_state != ska::pst::lmc::ObsState::SCANNING) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received end scan request but not in SCANNING state. Currently in {} state.", curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::NOT_SCANNING);

        std::ostringstream ss;
        ss << _service_name << " is not in SCANNING state. Currently in " << curr_state_name << " state.";
        status.set_message(ss.str());

        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        handler->end_scan();
        set_state(ska::pst::lmc::ObsState::READY);
        return grpc::Status::OK;
    } catch (std::exception& exc) {
        // handle exception
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INTERNAL_ERROR);
        status.set_message("Error in stopping scan request.");
        return grpc::Status(grpc::StatusCode::INTERNAL, status.message(), status.SerializeAsString());
    }
}

auto ska::pst::common::LmcService::get_state(
    grpc::ServerContext* context,
    const ska::pst::lmc::GetStateRequest* /*request*/,
    ska::pst::lmc::GetStateResponse* response
) -> grpc::Status
{
    spdlog::trace("ska::pst::common::LmcService::get_state()");
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
        spdlog::warn("Received monitor but not in SCANNING state. Currently in {} state.", curr_state_name);
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
            spdlog::info("No longer in SCANNING state. Exiting monitor");
            break;
        }
        spdlog::trace("Getting latest monitor data");

        ska::pst::lmc::MonitorResponse response;
        auto *monitor_data = response.mutable_monitor_data();
        handler->get_monitor_data(monitor_data);

        if (context->IsCancelled()) {
            spdlog::info("Monitoring context cancelled. Exiting monitor.");
            break;
        }
        if (!writer->Write(response)) {
            spdlog::warn("Writing monitor response return false. Exiting monitor.");
            break;
        }
    }

    return grpc::Status::OK;
}

auto ska::pst::common::LmcService::abort(
    grpc::ServerContext* context,
    const ska::pst::lmc::AbortRequest* /*request*/,
    ska::pst::lmc::AbortResponse* /*response*/
) -> grpc::Status
{
    if (_state == ska::pst::lmc::ObsState::ABORTED)
    {
        spdlog::warn("Received abort request but already in ABORTED state.");
        return grpc::Status::OK;
    }

    if (!(
        _state == ska::pst::lmc::ObsState::IDLE ||
        _state == ska::pst::lmc::ObsState::READY ||
        _state == ska::pst::lmc::ObsState::SCANNING
    )) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received abort request but not in an abortable state. Currently in {} state.",
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
            handler->end_scan();
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
    grpc::ServerContext* context,
    const ska::pst::lmc::ResetRequest* /*request*/,
    ska::pst::lmc::ResetResponse* /*response*/
) -> grpc::Status
{
    if (!(_state == ska::pst::lmc::ObsState::ABORTED or _state == ska::pst::lmc::ObsState::FAULT)) {
        // LMC is the source of truth, but we should have been moved to an ABORTED or FAULT state
        // before this could have been called.
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received reset request but not ABORTED or FAULT state. Currently in {} state.",
            curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in ABORTED or FAULT state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        if (handler->is_configured())
        {
            handler->deconfigure();
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
    grpc::ServerContext* context,
    const ska::pst::lmc::RestartRequest* /*request*/,
    ska::pst::lmc::RestartResponse* /*response*/
) -> grpc::Status
{
    if (!(_state == ska::pst::lmc::ObsState::ABORTED or _state == ska::pst::lmc::ObsState::FAULT)) {
        auto curr_state_name = ska::pst::lmc::ObsState_Name(_state);
        spdlog::warn("Received reset request but not ABORTED or FAULT state. Currently in {} state.",
            curr_state_name);
        ska::pst::lmc::Status status;
        status.set_code(ska::pst::lmc::ErrorCode::INVALID_REQUEST);

        std::ostringstream ss;
        ss << _service_name << " is not in ABORTED or FAULT state. Currently in " << curr_state_name << " state.";

        status.set_message(ss.str());
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, status.message(), status.SerializeAsString());
    }

    try {
        if (handler->is_configured())
        {
            handler->deconfigure();
        }
        if (handler->are_resources_assigned())
        {
            handler->release_resources();
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
