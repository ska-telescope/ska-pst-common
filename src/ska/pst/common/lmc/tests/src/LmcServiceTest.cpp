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
#include <spdlog/spdlog.h>
#include <grpc/grpc.h>
#include <grpc++/grpc++.h>
#include <google/protobuf/util/message_differencer.h>

#include "ska/pst/common/lmc/tests/LmcServiceTest.h"
#include "ska/pst/common/testutils/GtestMain.h"

#include "ska/pst/common/statemodel/StateModel.h"
#include "ska/pst/common/statemodel/StateModelException.h"

using namespace google::protobuf::util;

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

LmcServiceTest::LmcServiceTest()
    : ::testing::Test()
{
}

void LmcServiceTest::SetUp()
{
    SPDLOG_TRACE("LmcServiceTest::SetUp()");
    SPDLOG_TRACE("LmcServiceTest::SetUp creating shared data block manager");
    _handler = std::make_shared<TestLmcServiceHandler>();

    SPDLOG_TRACE("LmcServiceTest::SetUp creating shared data block manager");
    _service = std::make_shared<ska::pst::common::LmcService>("TEST", _handler, _port);

    // force getting a port set, we need gRPC to start to bind to get port.
    _service->start();

    _port = _service->port();

    std::string server_address("127.0.0.1:");
    SPDLOG_TRACE("LmcServiceTest::SetUp creating client connection on port {}", _port);
    server_address.append(std::to_string(_port));
    _channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    _stub = ska::pst::lmc::PstLmcService::NewStub(_channel);
}

void LmcServiceTest::TearDown()
{
    _service->stop();
}

auto LmcServiceTest::configure_beam(bool dry_run) -> grpc::Status
{
    ska::pst::lmc::ConfigureBeamRequest request;

    request.set_dry_run(dry_run);
    auto resources = request.mutable_beam_configuration();
    auto test_resources = resources->mutable_test();
    auto values = test_resources->mutable_resources();

    (*values)["foo"] = "bar";

    return configure_beam(request);
}

auto LmcServiceTest::configure_beam(const ska::pst::lmc::ConfigureBeamRequest& request) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::ConfigureBeamResponse response;
    try
    {
        if (_handler->induce_configure_beam_error)
        {
            SPDLOG_WARN("LmcServiceTest::configure_beam induced configure_beam error");
            _handler->set_state(ska::pst::common::RuntimeError);
            throw std::runtime_error("induced configure_beam error");
        }
    }
    catch(const std::exception& e)
    {
        _handler->go_to_runtime_error(std::current_exception());
    }
    return _stub->configure_beam(&context, request, &response);
}

auto LmcServiceTest::get_beam_configuration(
    ska::pst::lmc::GetBeamConfigurationResponse* response
) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::GetBeamConfigurationRequest request;

    return _stub->get_beam_configuration(&context, request, response);
}

auto LmcServiceTest::deconfigure_beam() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::DeconfigureBeamRequest request;
    ska::pst::lmc::DeconfigureBeamResponse response;
    try
    {
        if (_handler->induce_deconfigure_beam_error)
        {
            SPDLOG_WARN("LmcServiceTest::deconfigure_beam induced deconfigure_beam error");
            _handler->set_state(ska::pst::common::RuntimeError);
            throw std::runtime_error("induced deconfigure_beam error");
        }
    }
    catch(const std::exception& e)
    {
        _handler->go_to_runtime_error(std::current_exception());
    }

    return _stub->deconfigure_beam(&context, request, &response);
}

auto LmcServiceTest::configure_scan(bool dry_run) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::ConfigureScanRequest request;
    request.set_dry_run(dry_run);

    auto scan_configuration = request.mutable_scan_configuration();
    auto test_configuration = scan_configuration->mutable_test();
    auto values = test_configuration->mutable_configuration();

    (*values)["cat"] = "dog";

    ska::pst::lmc::ConfigureScanResponse response;

    return _stub->configure_scan(&context, request, &response);
}

auto LmcServiceTest::deconfigure_scan() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::DeconfigureScanRequest request;
    ska::pst::lmc::DeconfigureScanResponse response;

    return _stub->deconfigure_scan(&context, request, &response);
}

auto LmcServiceTest::get_scan_configuration(
    ska::pst::lmc::GetScanConfigurationResponse *response
) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::GetScanConfigurationRequest request;

    return _stub->get_scan_configuration(&context, request, response);
}

auto LmcServiceTest::start_scan() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::StartScanRequest request;
    ska::pst::lmc::StartScanResponse response;

    return _stub->start_scan(&context, request, &response);
}

auto LmcServiceTest::stop_scan() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::StopScanRequest request;
    ska::pst::lmc::StopScanResponse response;

    return _stub->stop_scan(&context, request, &response);
}

auto LmcServiceTest::abort() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::AbortRequest request;
    ska::pst::lmc::AbortResponse response;

    return _stub->abort(&context, request, &response);
}

auto LmcServiceTest::reset() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::ResetRequest request;
    ska::pst::lmc::ResetResponse response;

    return _stub->reset(&context, request, &response);
}

auto LmcServiceTest::restart() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::RestartRequest request;
    ska::pst::lmc::RestartResponse response;

    return _stub->restart(&context, request, &response);
}

auto LmcServiceTest::go_to_fault(const std::string& error_message) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::GoToFaultRequest request;
    request.set_error_message(error_message);
    ska::pst::lmc::GoToFaultResponse response;

    return _stub->go_to_fault(&context, request, &response);
}

auto LmcServiceTest::get_state(
    ska::pst::lmc::GetStateResponse* response
) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::GetStateRequest request;

    return _stub->get_state(&context, request, response);
}

auto LmcServiceTest::get_env(
    ska::pst::lmc::GetEnvironmentResponse* response
) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::GetEnvironmentRequest request;

    return _stub->get_env(&context, request, response);
}

void LmcServiceTest::assert_state(
    ska::pst::lmc::ObsState expected_state
)
{
    ska::pst::lmc::GetStateResponse get_state_response;
    auto status = get_state(&get_state_response);
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_EQ(get_state_response.state(), expected_state); // NOLINT
}

TEST_F(LmcServiceTest, connect) // NOLINT
{
    _service->start();
    EXPECT_TRUE(_service->is_running());

    grpc::ClientContext context;
    ska::pst::lmc::ConnectionRequest request;
    ska::pst::lmc::ConnectionResponse response;

    request.set_client_id("test_client_id");

    auto status = _stub->connect(&context, request, &response);

    EXPECT_TRUE(status.ok());
}

TEST_F(LmcServiceTest, start_stop) // NOLINT
{
    _service->start();

    EXPECT_TRUE(_service->is_running());

    _service->stop();

    EXPECT_FALSE(_service->is_running());
}

TEST_F(LmcServiceTest, configure_beam) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, get_beam_configuration);
    EXPECT_CALL(*_handler, deconfigure_beam);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::configure_beam - configuring beam");
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::configure_beam - beam configured");

    SPDLOG_TRACE("LmcServiceTest::configure_beam - getting beam configuration");
    ska::pst::lmc::GetBeamConfigurationResponse get_response;

    status = get_beam_configuration(&get_response);
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(get_response.has_beam_configuration());
    EXPECT_TRUE(get_response.beam_configuration().has_test());
    EXPECT_EQ(get_response.beam_configuration().DebugString(), _handler->resources.DebugString());

    status = deconfigure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    EXPECT_EQ(_handler->resources.DebugString(), ska::pst::lmc::BeamConfiguration().DebugString());

    assert_state(ska::pst::lmc::ObsState::EMPTY);
}

TEST_F(LmcServiceTest, configure_beam_during_dry_run) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::configure_beam_during_dry_run - configuring beam");
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    auto status = configure_beam(true);

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::configure_beam_during_dry_run - configured beam validated");
}

TEST_F(LmcServiceTest, configure_beam_when_already_assigned) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam).Times(1);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT

    status = configure_beam();

    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " beam configured already. Beam configuation needs to be deconfigured before reconfiguring.",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::CONFIGURED_FOR_BEAM_ALREADY, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, configure_beam_with_invalid_request) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam)
      .Times(1)
      .WillRepeatedly(testing::Throw(ska::pst::common::pst_validation_error("oops the request was invalid")));

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    auto status = configure_beam();
    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("Error in configuring beam: validation error - oops the request was invalid",
        status.error_message());  // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, configure_beam_with_invalid_request_during_dry_run) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam)
      .Times(1)
      .WillRepeatedly(testing::Throw(ska::pst::common::pst_validation_error("oops the request was invalid")));

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    auto status = configure_beam(true);
    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("Error in configuring beam: validation error - oops the request was invalid",
        status.error_message());  // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}


TEST_F(LmcServiceTest, get_beam_configuration_when_not_beam_configured) // NOLINT
{
    EXPECT_CALL(*_handler, get_beam_configuration).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    ska::pst::lmc::GetBeamConfigurationResponse response;
    auto status = get_beam_configuration(&response);

    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("No " + _service->service_name() + " beam configured.", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_CONFIGURED_FOR_BEAM, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, release_assigned_resources_when_not_beam_configured) // NOLINT
{
    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    auto status = deconfigure_beam();

    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("No " + _service->service_name() + " beam configured.", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_CONFIGURED_FOR_BEAM, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, configure_deconfigure) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, get_scan_configuration);
    EXPECT_CALL(*_handler, deconfigure_scan);
    EXPECT_CALL(*_handler, deconfigure_beam);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - configuring beam");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - beam configured");

    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - configuring scan");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - scan configured");

    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - getting configuration");
    ska::pst::lmc::GetScanConfigurationResponse get_response;

    status = get_scan_configuration(&get_response);

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(get_response.has_scan_configuration()); // NOLINT
    EXPECT_TRUE(get_response.scan_configuration().has_test()); // NOLINT
    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - checked configuration");

    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - deconfiguring scan");
    status = deconfigure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - scan deconfigured");

    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - deconfiguring beam");
    status = deconfigure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::configure_deconfigure - beam deconfigured");
}

// TODO - handle the error states of scan configuration methods.
// configure when not IDLE

TEST_F(LmcServiceTest, configure_when_not_idle) // NOLINT
{
    EXPECT_CALL(*_handler, configure_scan).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::configure_when_not_idle - configuring scan");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    auto status = configure_scan();
    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in IDLE state. Currently in EMPTY state.",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT

    assert_state(ska::pst::lmc::ObsState::EMPTY);
}

TEST_F(LmcServiceTest, configure_when_already_configured) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan).Times(1);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::configure_when_already_configured - configuring beam");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::configure_when_already_configured - beam configured");

    SPDLOG_TRACE("LmcServiceTest::configure_when_already_configured - configuring scan");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::configure_when_already_configured - scan configured");

    SPDLOG_TRACE("LmcServiceTest::configure_when_already_configured - configuring scan");
    status = configure_scan();
    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " already configured for scan. Scan needs to be deconfigured before reconfiguring.",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::CONFIGURED_FOR_SCAN_ALREADY, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT

    assert_state(ska::pst::lmc::ObsState::READY);
}

TEST_F(LmcServiceTest, configure_scan_with_invalid_request) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan)
      .Times(1)
      .WillRepeatedly(testing::Throw(ska::pst::common::pst_validation_error("this is a bad scan config")));

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::configure_scan_with_invalid_request - configuring beam");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::configure_scan_with_invalid_request - beam configured");

    SPDLOG_TRACE("LmcServiceTest::configure_scan_with_invalid_request - configuring scan");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    status = configure_scan();
    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("Error in configuring scan: validation error - this is a bad scan config",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, get_scan_configuration_when_not_ready_or_scanning) // NOLINT
{
    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_when_not_ready_or_scanning - getting scan configuration");
    ska::pst::lmc::GetScanConfigurationResponse get_response;

    auto status = get_scan_configuration(&get_response);
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in a configured state. Currently in EMPTY state.",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, deconfigure_when_not_ready_state) // NOLINT
{
    EXPECT_CALL(*_handler, configure_scan).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::deconfigure_when_not_ready_state - configuring scan");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    auto status = deconfigure_scan();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in READY state. Currently in EMPTY state.",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT

    assert_state(ska::pst::lmc::ObsState::EMPTY);
}

TEST_F(LmcServiceTest, scan_endscan) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, stop_scan);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::scan_endscan - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::scan_endscan - beam configured");

    SPDLOG_TRACE("LmcServiceTest::scan_endscan - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::scan_endscan - scan configured");

    SPDLOG_TRACE("LmcServiceTest::scan_endscan - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::scan_endscan - scanning");

    SPDLOG_TRACE("LmcServiceTest::scan_endscan - ending scan");
    status = stop_scan();
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
}

TEST_F(LmcServiceTest, get_scan_configuration_while_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, get_scan_configuration);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - beam configured");

    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::scan_endscan - scan configured");

    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - scanning");

    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - get scan configuration");
    ska::pst::lmc::GetScanConfigurationResponse get_response;

    status = get_scan_configuration(&get_response);

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(get_response.has_scan_configuration()); // NOLINT
    EXPECT_TRUE(get_response.scan_configuration().has_test()); // NOLINT
    SPDLOG_TRACE("LmcServiceTest::get_scan_configuration_while_scanning - checked configuration");
}

TEST_F(LmcServiceTest, scan_when_already_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan).Times(1);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::scan_when_already_scanning - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::scan_when_already_scanning - beam configured");

    SPDLOG_TRACE("LmcServiceTest::scan_when_already_scanning - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::scan_when_already_scanning - scan configured");

    SPDLOG_TRACE("LmcServiceTest::scan_when_already_scanning - starting scan");
    status = start_scan();
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::scan_when_already_scanning - scanning");

    status = start_scan();

    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is already scanning.", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::ALREADY_SCANNING, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, scan_when_not_ready) // NOLINT
{
    EXPECT_CALL(*_handler, start_scan).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::scan_when_not_ready - starting scan");

    auto status = start_scan();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in READY state. Currently in EMPTY state.", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, scan_when_validation_fails) // NOLINT
{
    EXPECT_CALL(*_handler, start_scan)
      .Times(1)
      .WillRepeatedly(testing::Throw(ska::pst::common::pst_validation_error("this is not a valid scan request")));

    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::scan_when_validation_fails - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::scan_when_validation_fails - beam configured");

    SPDLOG_TRACE("LmcServiceTest::scan_when_validation_fails - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::scan_when_validation_fails - scan configured");

    SPDLOG_TRACE("LmcServiceTest::scan_when_validation_fails - starting scan");
    status = start_scan();
    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("Error in starting scan: validation error - this is not a valid scan request", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, stop_scan_when_not_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, stop_scan).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::stop_scan_when_not_scanning - calling stop_scan");
    auto status = stop_scan();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in SCANNING state. Currently in EMPTY state.", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_SCANNING, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, abort_when_in_idle_state) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - beam configured");

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);
}

TEST_F(LmcServiceTest, abort_when_in_ready_state) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - beam configured");

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - scan configured");

    SPDLOG_TRACE("LmcServiceTest::abort_when_in_ready_state - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);
}

TEST_F(LmcServiceTest, abort_when_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, stop_scan);

    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning - beam configured");

    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning - scan configured");

    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning - starting scan");
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);

    SPDLOG_TRACE("LmcServiceTest::abort_when_scanning - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT

    ska::pst::lmc::GetStateResponse get_state_response;
    status = get_state(&get_state_response);
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_EQ(get_state_response.state(), ska::pst::lmc::ObsState::ABORTED); // NOLINT
}

TEST_F(LmcServiceTest, abort_when_not_in_abortable_state) // NOLINT
{
    SPDLOG_TRACE("LmcServiceTest::abort_when_not_in_abortable_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::abort_when_not_in_abortable_state - aborting");
    auto status = abort();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in an abortable state. Currently in EMPTY state.",
        status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, abort_when_already_in_aborted_state) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);

    SPDLOG_TRACE("LmcServiceTest::abort_when_already_in_aborted_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::abort_when_already_in_aborted_state - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::abort_when_already_in_aborted_state - beam configured");

    SPDLOG_TRACE("LmcServiceTest::abort_when_already_in_aborted_state - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);
}

TEST_F(LmcServiceTest, reset_when_aborted) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, reset);
    EXPECT_CALL(*_handler, deconfigure_beam);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted - beam configured");

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted - resetting");
    status = reset();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
}

TEST_F(LmcServiceTest, reset_when_aborted_and_scan_configured) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, deconfigure_scan);
    EXPECT_CALL(*_handler, deconfigure_beam);
    EXPECT_CALL(*_handler, reset);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - beam configured");

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - configuring scan");
    status = configure_scan();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - scan configured");

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - resetting");
    status = reset();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - reset");
}

TEST_F(LmcServiceTest, reset_when_empty) // NOLINT
{
    EXPECT_CALL(*_handler, reset);

    SPDLOG_TRACE("LmcServiceTest::reset_when_empty");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::reset_when_empty - resetting");
    auto status = reset();
    EXPECT_FALSE(_handler->is_beam_configured());
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::reset_when_empty - service reset");
}

TEST_F(LmcServiceTest, reset_when_not_aborted_or_fault) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, deconfigure_scan);
    EXPECT_CALL(*_handler, deconfigure_beam);
    EXPECT_CALL(*_handler, reset).Times(2);

    SPDLOG_TRACE("LmcServiceTest::reset_when_not_aborted_or_fault");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::reset_when_not_aborted_or_fault - resetting");
    auto status = reset();
    EXPECT_TRUE(status.ok()); // NOLINT

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - configuring beam");
    status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - beam configured");

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - configuring scan");
    status = configure_scan();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);

    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - resetting");
    status = reset();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::reset_when_aborted_and_scan_configured - reset");
}

TEST_F(LmcServiceTest, restart_when_aborted) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, deconfigure_beam);

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted - beam configured");

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted - restarting");
    status = restart();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
}

TEST_F(LmcServiceTest, restart_when_aborted_and_scan_configured) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, deconfigure_scan);
    EXPECT_CALL(*_handler, deconfigure_beam);

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - beam configured");

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - configuring scan");
    status = configure_scan();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - scan configured");

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);
    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - aborted");

    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - restarting");
    status = restart();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured());
    EXPECT_FALSE(_handler->is_scan_configured());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::restart_when_aborted_and_scan_configured - restarted");
}

TEST_F(LmcServiceTest, restart_when_empty) // NOLINT
{
    SPDLOG_TRACE("LmcServiceTest::restart_when_empty");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::restart_when_empty - restarting");
    auto status = restart();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::restart_when_empty - restarted");
}

TEST_F(LmcServiceTest, restart_when_not_aborted_or_fault) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);

    SPDLOG_TRACE("LmcServiceTest::restart_when_not_aborted_or_fault");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::restart_when_not_aborted_or_fault - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::restart_when_not_aborted_or_fault - beam configured");

    SPDLOG_TRACE("LmcServiceTest::restart_when_not_aborted_or_fault - restarting");
    status = restart();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in ABORTED or FAULT state. Currently in IDLE state.",
        status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

// test can only monitor if in scanning state
TEST_F(LmcServiceTest, monitor_only_when_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, get_monitor_data).Times(testing::AtLeast(1));

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - calling monitor");

    // Check monitor
    grpc::ClientContext monitor_context1;
    ska::pst::lmc::MonitorRequest monitor_request;
    monitor_request.set_polling_rate(100);
    std::unique_ptr<grpc::ClientReader<ska::pst::lmc::MonitorResponse>> monitor_response_reader(
        _stub->monitor(&monitor_context1, monitor_request)
    );

    monitor_response_reader->WaitForInitialMetadata();

    ska::pst::lmc::MonitorResponse monitor_response;
    EXPECT_FALSE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    auto monitor_response_status = monitor_response_reader->Finish();

    EXPECT_FALSE(monitor_response_status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, monitor_response_status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in SCANNING state. Currently in EMPTY state.", monitor_response_status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(monitor_response_status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_SCANNING, lmc_status.code()); // NOLINT
    EXPECT_EQ(monitor_response_status.error_message(), lmc_status.message()); // NOLINT

    // configure beam
    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - beam configured");

    // Check monitor
    grpc::ClientContext monitor_context2;
    monitor_response_reader = _stub->monitor(&monitor_context2, monitor_request);
    monitor_response_reader->WaitForInitialMetadata();

    EXPECT_FALSE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    monitor_response_status = monitor_response_reader->Finish();

    EXPECT_FALSE(monitor_response_status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, monitor_response_status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in SCANNING state. Currently in IDLE state.", monitor_response_status.error_message()); // NOLINT

    lmc_status.ParseFromString(monitor_response_status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_SCANNING, lmc_status.code()); // NOLINT
    EXPECT_EQ(monitor_response_status.error_message(), lmc_status.message()); // NOLINT

    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - scan configured");

    grpc::ClientContext monitor_context3;
    monitor_response_reader = _stub->monitor(&monitor_context3, monitor_request);
    monitor_response_reader->WaitForInitialMetadata();

    EXPECT_FALSE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    monitor_response_status = monitor_response_reader->Finish();

    EXPECT_FALSE(monitor_response_status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, monitor_response_status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in SCANNING state. Currently in READY state.", monitor_response_status.error_message()); // NOLINT

    lmc_status.ParseFromString(monitor_response_status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_SCANNING, lmc_status.code()); // NOLINT
    EXPECT_EQ(monitor_response_status.error_message(), lmc_status.message()); // NOLINT

    // scan
    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - starting scan");
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - scanning");

    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - starting to monitor");
    // monitor
    grpc::ClientContext monitor_context4;
    monitor_response_reader = _stub->monitor(&monitor_context4, monitor_request);
    monitor_response_reader->WaitForInitialMetadata();

    EXPECT_TRUE(monitor_response_reader->Read(&monitor_response)); // NOLINT

    EXPECT_TRUE(monitor_response.has_monitor_data());
    EXPECT_TRUE(monitor_response.monitor_data().has_test());

    monitor_context4.TryCancel();
    monitor_response_status = monitor_response_reader->Finish();
    EXPECT_FALSE(monitor_response_status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::CANCELLED, monitor_response_status.error_code()); // NOLINT
    SPDLOG_TRACE("LmcServiceTest::monitor_only_when_scanning - end monitoring");
}

// test stopping of scanning should stop monitoring
TEST_F(LmcServiceTest, monitor_should_stop_when_scanning_stops) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, get_monitor_data).Times(testing::AtLeast(1));
    EXPECT_CALL(*_handler, stop_scan);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    // configure beam
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - configuring beam");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - resources allocated");

    // configure
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - scan configured");

    // scan
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - starting scan");
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - scanning");

    // monitor
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - starting to monitor");
    grpc::ClientContext monitor_context;
    ska::pst::lmc::MonitorRequest monitor_request;
    ska::pst::lmc::MonitorResponse monitor_response;
    monitor_request.set_polling_rate(100);
    std::unique_ptr<grpc::ClientReader<ska::pst::lmc::MonitorResponse>> monitor_response_reader(
        _stub->monitor(&monitor_context, monitor_request)
    );

    monitor_response_reader->WaitForInitialMetadata();

    EXPECT_TRUE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - monitoring");

    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - ending scan");
    // end scan
    grpc::ClientContext stop_scan_context;
    ska::pst::lmc::StopScanRequest stop_scan_request;
    ska::pst::lmc::StopScanResponse stop_scan_response;
    EXPECT_TRUE(_stub->stop_scan(&stop_scan_context, stop_scan_request, &stop_scan_response).ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - scan ended");

    SPDLOG_TRACE("LmcServiceTest::monitor_should_stop_when_scanning_stops - checking monitoring respons");
    EXPECT_FALSE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    const auto &monitor_response_status = monitor_response_reader->Finish();
    EXPECT_TRUE(monitor_response_status.ok()); // NOLINT
}

TEST_F(LmcServiceTest, go_to_fault_when_not_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, stop_scan).Times(0);
    EXPECT_CALL(*_handler, go_to_runtime_error);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_not_scanning - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_not_scanning - beam configured");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_not_scanning - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_not_scanning - scan configured");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_not_scanning - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = go_to_fault("test fault message");
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

TEST_F(LmcServiceTest, go_to_fault_when_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, stop_scan);
    EXPECT_CALL(*_handler, go_to_runtime_error);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - beam configured");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - scan configured");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - scanning");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning - go to fault");
    status = go_to_fault("test going to a fault state");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

TEST_F(LmcServiceTest, go_to_fault_when_scanning_throws_exception) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, stop_scan)
        .Times(1)
        .WillRepeatedly(testing::Throw(std::exception()));
    EXPECT_CALL(*_handler, go_to_runtime_error);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - configuring beam");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - beam configured");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - configuring scan");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - scan configured");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - scanning");

    SPDLOG_TRACE("LmcServiceTest::go_to_fault_when_scanning_throws_exception - go to fault");
    status = go_to_fault("test going to fault when scanning");
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

TEST_F(LmcServiceTest, get_env_with_default) // NOLINT
{
    EXPECT_CALL(*_handler, get_env);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT

    ska::pst::lmc::GetEnvironmentResponse response;
    auto status = get_env(&response);

    EXPECT_EQ(response.values().size(), 0);
}

TEST_F(LmcServiceTest, get_env_with_implementation) // NOLINT
{
    auto string_value = ska::pst::lmc::EnvValue();
    string_value.set_string_value("somestring");
    auto float_value = ska::pst::lmc::EnvValue();
    float_value.set_float_value(3.14);
    auto unsigned_int_value = ska::pst::lmc::EnvValue();
    unsigned_int_value.set_unsigned_int_value(1234);
    auto signed_int_value = ska::pst::lmc::EnvValue();
    signed_int_value.set_signed_int_value(-4321);

    EXPECT_CALL(*_handler, get_env)
        .WillOnce([this, string_value, float_value, unsigned_int_value, signed_int_value](ska::pst::lmc::GetEnvironmentResponse *response) {
                auto values = response->mutable_values();
                (*values)["string_value"] = string_value;
                (*values)["float_value"] = float_value;
                (*values)["unsigned_int_value"] = unsigned_int_value;
                (*values)["signed_int_value"] = signed_int_value;
            });

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT

    ska::pst::lmc::GetEnvironmentResponse response;
    auto status = get_env(&response);

    EXPECT_EQ(response.values().size(), 4);
    auto values = response.values();

    EXPECT_TRUE(MessageDifferencer::Equals(values.at("string_value"), string_value));
    EXPECT_TRUE(MessageDifferencer::Equals(values.at("float_value"), float_value));
    EXPECT_TRUE(MessageDifferencer::Equals(values.at("unsigned_int_value"), unsigned_int_value));
    EXPECT_TRUE(MessageDifferencer::Equals(values.at("signed_int_value"), signed_int_value));
}

TEST_F(LmcServiceTest, go_to_fault_when_runtime_error_encountered_configure_beam) // NOLINT
{
    EXPECT_CALL(*_handler, go_to_runtime_error);
    _handler->induce_configure_beam_error=true;

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::configure_beam - configuring beam");
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    auto status = configure_beam();
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

TEST_F(LmcServiceTest, go_to_fault_when_runtime_error_encountered_deconfigure_beam) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, go_to_runtime_error);
    _handler->induce_deconfigure_beam_error=true;

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    SPDLOG_TRACE("LmcServiceTest::configure_beam - configuring beam");
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLIN
    auto status = configure_beam();
    status = deconfigure_beam();
    ASSERT_EQ(
        status.error_message(),
        "RuntimeError before deconfiguring beam: induced deconfigure_beam error"
    );
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

TEST_F(LmcServiceTest, rethrow_application_manager_runtime_error) // NOLINT
{
    EXPECT_CALL(*_handler, go_to_runtime_error);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    auto status = go_to_fault("test going to fault when scanning");
}

} // namespace ska::pst::common::test