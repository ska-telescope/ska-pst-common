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

#include "ska/pst/common/lmc/tests/LmcServiceTest.h"
#include "ska/pst/common/testutils/GtestMain.h"

auto main(int argc, char* argv[]) -> int
{
  spdlog::set_level(spdlog::level::trace);
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska {
namespace pst {
namespace common {
namespace test {

LmcServiceTest::LmcServiceTest()
    : ::testing::Test()
{
}

void LmcServiceTest::SetUp()
{
    spdlog::trace("LmcServiceTest::SetUp()");
    spdlog::trace("LmcServiceTest::SetUp creating shared data block manager");
    _handler = std::make_shared<TestLmcServiceHandler>();

    spdlog::trace("LmcServiceTest::SetUp creating shared data block manager");
    _service = std::make_shared<ska::pst::common::LmcService>("TEST", _handler, _port);

    // force getting a port set, we need gRPC to start to bind to get port.
    _service->start();

    _port = _service->port();

    std::string server_address("127.0.0.1:");
    spdlog::trace("LmcServiceTest::SetUp creating client connection on port {}", _port);
    server_address.append(std::to_string(_port));
    _channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    _stub = ska::pst::lmc::PstLmcService::NewStub(_channel);
}

void LmcServiceTest::TearDown()
{
    _service->stop();
}

auto LmcServiceTest::configure_beam() -> grpc::Status
{
    ska::pst::lmc::ConfigureBeamRequest request;

    auto resources = request.mutable_beam_configuration();
    auto test_resources = resources->mutable_test();
    auto values = test_resources->mutable_resources();

    (*values)["foo"] = "bar";

    return configure_beam(request);
}

auto LmcServiceTest::configure_beam(ska::pst::lmc::ConfigureBeamRequest request) -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::ConfigureBeamResponse response;

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

    return _stub->deconfigure_beam(&context, request, &response);
}

auto LmcServiceTest::configure_scan() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::ConfigureScanRequest request;

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

auto LmcServiceTest::go_to_fault() -> grpc::Status
{
    grpc::ClientContext context;
    ska::pst::lmc::GoToFaultRequest request;
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

    spdlog::trace("LmcServiceTest::configure_beam - assigning resources");
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::configure_beam - resources assigned");

    spdlog::trace("LmcServiceTest::configure_beam - getting assigned resources");
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
    EXPECT_EQ(_service->service_name() + " resources already assigned. Resources need to be released before reassigning.",
        status.error_message());  // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::CONFIGURED_FOR_BEAM_ALREADY, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, get_beam_configuration_when_no_resources_assigned) // NOLINT
{
    EXPECT_CALL(*_handler, get_beam_configuration).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    ska::pst::lmc::GetBeamConfigurationResponse response;
    auto status = get_beam_configuration(&response);

    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("No " + _service->service_name() + " resources assigned.", status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::NOT_CONFIGURED_FOR_BEAM, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, release_assigned_resources_when_no_resources_assigned) // NOLINT
{
    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    auto status = deconfigure_beam();

    EXPECT_FALSE(status.ok()); // NOLINT
    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ("No " + _service->service_name() + " resources assigned.", status.error_message()); // NOLINT

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

    spdlog::trace("LmcServiceTest::configure_deconfigure - assigning resources");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::configure_deconfigure - resources assigned");

    spdlog::trace("LmcServiceTest::configure_deconfigure - configuring");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::configure_deconfigure - configured");

    spdlog::trace("LmcServiceTest::configure_deconfigure - getting configuration");
    ska::pst::lmc::GetScanConfigurationResponse get_response;

    status = get_scan_configuration(&get_response);

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(get_response.has_scan_configuration()); // NOLINT
    EXPECT_TRUE(get_response.scan_configuration().has_test()); // NOLINT
    spdlog::trace("LmcServiceTest::configure_deconfigure - checked configuration");

    spdlog::trace("LmcServiceTest::configure_deconfigure - deconfiguring");
    status = deconfigure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::configure_deconfigure - deconfigured");

    spdlog::trace("LmcServiceTest::configure_deconfigure - releasing resources");
    status = deconfigure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    spdlog::trace("LmcServiceTest::configure_deconfigure - resources released");
}

// TODO - handle the error states of scan configuration methods.
// configure when not IDLE

TEST_F(LmcServiceTest, configure_when_not_idle) // NOLINT
{
    EXPECT_CALL(*_handler, configure_scan).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::configure_when_not_idle - configuring");
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

    spdlog::trace("LmcServiceTest::configure_when_already_configured - assigning resources");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::configure_when_already_configured - resources assigned");

    spdlog::trace("LmcServiceTest::configure_when_already_configured - configuring");
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::configure_when_already_configured - configured");

    spdlog::trace("LmcServiceTest::configure_when_already_configured - configuring");
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


TEST_F(LmcServiceTest, get_scan_configuration_when_not_ready_or_scanning) // NOLINT
{
    _service->start();
    EXPECT_TRUE(_service->is_running());
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::get_scan_configuration_when_not_ready_or_scanning - getting scan configuration");
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

    spdlog::trace("LmcServiceTest::deconfigure_when_not_ready_state - configuring");
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

    spdlog::trace("LmcServiceTest::scan_endscan - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::scan_endscan - resources assigned");

    spdlog::trace("LmcServiceTest::scan_endscan - configuring");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::scan_endscan - configured");

    spdlog::trace("LmcServiceTest::scan_endscan - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::scan_endscan - scanning");

    spdlog::trace("LmcServiceTest::scan_endscan - ending scan");
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

    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - resources assigned");

    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - configuring");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::scan_endscan - configured");

    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - scanning");

    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - get scan configuration");
    ska::pst::lmc::GetScanConfigurationResponse get_response;

    status = get_scan_configuration(&get_response);

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(get_response.has_scan_configuration()); // NOLINT
    EXPECT_TRUE(get_response.scan_configuration().has_test()); // NOLINT
    spdlog::trace("LmcServiceTest::get_scan_configuration_while_scanning - checked configuration");
}

TEST_F(LmcServiceTest, scan_when_already_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan).Times(1);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::scan_when_already_scanning - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::scan_when_already_scanning - resources assigned");

    spdlog::trace("LmcServiceTest::scan_when_already_scanning - configuring");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::scan_when_already_scanning - configured");

    spdlog::trace("LmcServiceTest::scan_when_already_scanning - starting scan");
    status = start_scan();
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::scan_when_already_scanning - scanning");

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

    spdlog::trace("LmcServiceTest::scan_when_not_ready - starting scan");

    auto status = start_scan();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in READY state. Currently in EMPTY state.", status.error_message()); // NOLINT

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

    spdlog::trace("LmcServiceTest::stop_scan_when_not_scanning - calling stop_scan");
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

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - resources assigned");

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);
}

TEST_F(LmcServiceTest, abort_when_in_ready_state) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - resources assigned");

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - configuring");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - configured");

    spdlog::trace("LmcServiceTest::abort_when_in_ready_state - aborting");
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

    spdlog::trace("LmcServiceTest::abort_when_scanning");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::abort_when_scanning - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::abort_when_scanning - resources assigned");

    spdlog::trace("LmcServiceTest::abort_when_scanning - configuring");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::abort_when_scanning - configured");

    spdlog::trace("LmcServiceTest::abort_when_scanning - starting scan");
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);

    spdlog::trace("LmcServiceTest::abort_when_scanning - aborting");
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
    spdlog::trace("LmcServiceTest::abort_when_not_in_abortable_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::abort_when_not_in_abortable_state - aborting");
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

    spdlog::trace("LmcServiceTest::abort_when_already_in_aborted_state");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::abort_when_already_in_aborted_state - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::abort_when_already_in_aborted_state - resources assigned");

    spdlog::trace("LmcServiceTest::abort_when_already_in_aborted_state - aborting");
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

    spdlog::trace("LmcServiceTest::reset_when_aborted");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::reset_when_aborted - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::reset_when_aborted - resources assigned");

    spdlog::trace("LmcServiceTest::reset_when_aborted - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    spdlog::trace("LmcServiceTest::reset_when_aborted - resetting");
    status = reset();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured());
    assert_state(ska::pst::lmc::ObsState::IDLE);
}

TEST_F(LmcServiceTest, reset_when_aborted_and_scan_configured) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, deconfigure_scan);

    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - resources assigned");

    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - configuring scan");
    status = configure_scan();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - configuring scan");

    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - resetting");
    status = reset();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured());
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::reset_when_aborted_and_scan_configured - reset");
}

TEST_F(LmcServiceTest, reset_when_idle) // NOLINT
{
    spdlog::trace("LmcServiceTest::reset_when_idle");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::reset_when_idle - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::reset_when_idle - resources assigned");


    spdlog::trace("LmcServiceTest::reset_when_idle - resetting");
    status = reset();
    EXPECT_TRUE(_handler->is_beam_configured());
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::restart_when_empty - service reset");
}

TEST_F(LmcServiceTest, reset_when_not_aborted_or_fault) // NOLINT
{
    spdlog::trace("LmcServiceTest::reset_when_not_aborted_or_fault");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::reset_when_not_aborted_or_fault - resetting");
    auto status = reset();
    EXPECT_FALSE(status.ok()); // NOLINT

    EXPECT_EQ(grpc::StatusCode::FAILED_PRECONDITION, status.error_code()); // NOLINT
    EXPECT_EQ(_service->service_name() + " is not in ABORTED or FAULT state. Currently in EMPTY state.",
        status.error_message()); // NOLINT

    ska::pst::lmc::Status lmc_status;
    lmc_status.ParseFromString(status.error_details());
    EXPECT_EQ(ska::pst::lmc::ErrorCode::INVALID_REQUEST, lmc_status.code()); // NOLINT
    EXPECT_EQ(status.error_message(), lmc_status.message()); // NOLINT
}

TEST_F(LmcServiceTest, restart_when_aborted) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, deconfigure_beam);

    spdlog::trace("LmcServiceTest::restart_when_aborted");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::restart_when_aborted - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::restart_when_aborted - resources assigned");

    spdlog::trace("LmcServiceTest::restart_when_aborted - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);

    spdlog::trace("LmcServiceTest::restart_when_aborted - restarting");
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

    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - resources assigned");

    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - configuring scan");
    status = configure_scan();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - configuring scan");

    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - aborting");
    status = abort();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::ABORTED);
    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - aborted");

    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - restarting");
    status = restart();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_FALSE(_handler->is_beam_configured());
    EXPECT_FALSE(_handler->is_scan_configured());
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    spdlog::trace("LmcServiceTest::restart_when_aborted_and_scan_configured - restarted");
}

TEST_F(LmcServiceTest, restart_when_empty) // NOLINT
{
    spdlog::trace("LmcServiceTest::restart_when_empty");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::restart_when_empty - restarting");
    auto status = restart();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);
    spdlog::trace("LmcServiceTest::restart_when_empty - restarted");
}

TEST_F(LmcServiceTest, restart_when_not_aborted_or_fault) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);

    spdlog::trace("LmcServiceTest::restart_when_not_aborted_or_fault");
    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::restart_when_not_aborted_or_fault - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::restart_when_not_aborted_or_fault - resources assigned");

    spdlog::trace("LmcServiceTest::restart_when_not_aborted_or_fault - restarting");
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

    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - calling monitor");

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

    // assign resources
    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - resources assigned");

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

    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - configuring");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - configured");

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
    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - starting scan");
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - scanning");

    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - starting to monitor");
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
    spdlog::trace("LmcServiceTest::monitor_only_when_scanning - end monitoring");
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

    // assign resources
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - assigning resources");
    auto status = configure_beam();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - resources allocated");

    // configure
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - configuring");
    status = configure_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - configured");

    // scan
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - starting scan");
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - scanning");

    // monitor
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - starting to monitor");
    grpc::ClientContext monitor_context;
    ska::pst::lmc::MonitorRequest monitor_request;
    ska::pst::lmc::MonitorResponse monitor_response;
    monitor_request.set_polling_rate(100);
    std::unique_ptr<grpc::ClientReader<ska::pst::lmc::MonitorResponse>> monitor_response_reader(
        _stub->monitor(&monitor_context, monitor_request)
    );

    monitor_response_reader->WaitForInitialMetadata();

    EXPECT_TRUE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - monitoring");

    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - ending scan");
    // end scan
    grpc::ClientContext stop_scan_context;
    ska::pst::lmc::StopScanRequest stop_scan_request;
    ska::pst::lmc::StopScanResponse stop_scan_response;
    EXPECT_TRUE(_stub->stop_scan(&stop_scan_context, stop_scan_request, &stop_scan_response).ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - scan ended");

    spdlog::trace("LmcServiceTest::monitor_should_stop_when_scanning_stops - checking monitoring respons");
    EXPECT_FALSE(monitor_response_reader->Read(&monitor_response)); // NOLINT
    const auto &monitor_response_status = monitor_response_reader->Finish();
    EXPECT_TRUE(monitor_response_status.ok()); // NOLINT
}

TEST_F(LmcServiceTest, go_to_fault_when_not_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, stop_scan).Times(0);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::go_to_fault_when_not_scanning - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::go_to_fault_when_not_scanning - resources assigned");

    spdlog::trace("LmcServiceTest::go_to_fault_when_not_scanning - configuring");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::go_to_fault_when_not_scanning - configured");

    spdlog::trace("LmcServiceTest::go_to_fault_when_not_scanning - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = go_to_fault();
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

TEST_F(LmcServiceTest, go_to_fault_when_scanning) // NOLINT
{
    EXPECT_CALL(*_handler, configure_beam);
    EXPECT_CALL(*_handler, configure_scan);
    EXPECT_CALL(*_handler, start_scan);
    EXPECT_CALL(*_handler, stop_scan);

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - resources assigned");

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - configuring");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - configured");

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - scanning");

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning - go to fault");
    status = go_to_fault();
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

    _service->start();
    EXPECT_TRUE(_service->is_running()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::EMPTY);

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - assigning resources");
    auto status = configure_beam();

    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_beam_configured()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::IDLE);
    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - resources assigned");

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - configuring");
    status = configure_scan();
    EXPECT_TRUE(_handler->is_scan_configured()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::READY);
    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - configured");

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - starting scan");
    EXPECT_FALSE(_handler->is_scanning()); // NOLINT
    status = start_scan();
    EXPECT_TRUE(status.ok()); // NOLINT
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::SCANNING);
    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - scanning");

    spdlog::trace("LmcServiceTest::go_to_fault_when_scanning_throws_exception - go to fault");
    status = go_to_fault();
    EXPECT_TRUE(_handler->is_scanning()); // NOLINT
    EXPECT_TRUE(status.ok()); // NOLINT
    assert_state(ska::pst::lmc::ObsState::FAULT);
}

} // namespace test
} // namespace common
} // namespace pst
} // namespace ska