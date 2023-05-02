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

#include <memory>
#include <grpc++/grpc++.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ska/pst/common/lmc/LmcService.h"
#include "ska/pst/common/lmc/LmcServiceHandler.h"
#include "ska/pst/lmc/ska_pst_lmc.grpc.pb.h"

#ifndef SKA_PST_SMRB_TESTS_LmcServiceTest_h
#define SKA_PST_SMRB_TESTS_LmcServiceTest_h

namespace ska::pst::common::test {

class TestLmcServiceHandler : public ska::pst::common::LmcServiceHandler {
    public:
        TestLmcServiceHandler() {
            ON_CALL(*this, configure_beam).WillByDefault([this](const ska::pst::lmc::BeamConfiguration &new_resources, bool dry_run) {
                if (!dry_run) {
                  resources.CopyFrom(new_resources);
                  beam_configured = true;
                }
            });
            ON_CALL(*this, deconfigure_beam).WillByDefault([this]() {
                resources.Clear();
                beam_configured = false;
            });
            ON_CALL(*this, get_beam_configuration).WillByDefault([this](ska::pst::lmc::BeamConfiguration *response) {
                response->CopyFrom(resources);
            });

            ON_CALL(*this, configure_scan).WillByDefault([this](const ska::pst::lmc::ScanConfiguration &configuration, bool dry_run) {
                if (!dry_run) {
                  scan_configuration.CopyFrom(configuration);
                  scan_configured = true;
                }
            });
            ON_CALL(*this, deconfigure_scan).WillByDefault([this]() {
                scan_configuration.Clear();
                scan_configured = false;
            });
            ON_CALL(*this, get_scan_configuration).WillByDefault([this](ska::pst::lmc::ScanConfiguration *configuration) {
                configuration->CopyFrom(scan_configuration);
            });


            ON_CALL(*this, start_scan).WillByDefault([this](const ska::pst::lmc::StartScanRequest &request) {
                scanning = true;
            });
            ON_CALL(*this, stop_scan).WillByDefault([this]() {
                scanning = false;
            });

            ON_CALL(*this, get_monitor_data).WillByDefault([this](ska::pst::lmc::MonitorData *data) {
                auto *test_monitor_data = data->mutable_test();
                auto *values = test_monitor_data->mutable_data();

                (*values)["hot"] = "cold";
            });

            ON_CALL(*this, get_env).WillByDefault([this](ska::pst::lmc::GetEnvironmentResponse *data) {
                ska::pst::common::LmcServiceHandler::get_env(data);
            });

            ON_CALL(*this, reset).WillByDefault([this]() {
                // this is the same as a deconfigure_scan and beam
                // but done directly to allow assertions against the mock
                set_state(ska::pst::common::State::Idle);
            });

            ON_CALL(*this, go_to_runtime_error).WillByDefault([this](std::exception_ptr exc) {
                _exception = exc;
                set_state(ska::pst::common::State::RuntimeError);
            });
        }

        // testing fields
        bool beam_configured{false};
        bool scan_configured{false};
        bool scanning{false};
        bool induce_configure_beam_error{false};
        bool induce_deconfigure_beam_error{false};
        ska::pst::lmc::BeamConfiguration resources{};
        ska::pst::lmc::ScanConfiguration scan_configuration{};
        ska::pst::common::State _state = ska::pst::common::Unknown;
        std::exception_ptr _exception = nullptr;

        // Resources
        MOCK_METHOD(void, configure_beam, (const ska::pst::lmc::BeamConfiguration &resources, bool dry_run), (override));
        MOCK_METHOD(void, deconfigure_beam, (), (override));
        MOCK_METHOD(void, get_beam_configuration, (ska::pst::lmc::BeamConfiguration *response), (override));
        bool is_beam_configured() const noexcept override {
            return beam_configured;
        }

        // Scan configuration
        MOCK_METHOD(void, configure_scan, (const ska::pst::lmc::ScanConfiguration &configuration, bool dry_run), (override));
        MOCK_METHOD(void, deconfigure_scan, (), (override));
        MOCK_METHOD(void, get_scan_configuration, (ska::pst::lmc::ScanConfiguration *configuration), (override));
        bool is_scan_configured() const noexcept override {
            return scan_configured;
        }

        // Scan methods
        MOCK_METHOD(void, start_scan, (const ska::pst::lmc::StartScanRequest &request), (override));
        MOCK_METHOD(void, stop_scan, (), (override));
        bool is_scanning() const noexcept override {
            return scanning;
        }

        // Monitor
        MOCK_METHOD(void, get_monitor_data, (ska::pst::lmc::MonitorData *data), (override));
        MOCK_METHOD(void, get_env, (ska::pst::lmc::GetEnvironmentResponse *data), (noexcept, override));

        // ERROR HANDLING
        MOCK_METHOD(void, reset, (), (override));
        MOCK_METHOD(void, go_to_runtime_error, (std::exception_ptr), (override));

        // Get ApplicationManager details
        ska::pst::common::State get_application_manager_state() { return _state; }
        std::exception_ptr get_application_manager_exception() { return _exception; }
        void set_state(ska::pst::common::State desired_state) { _state=desired_state; }
        void set_exception(std::exception_ptr desired_exception ) { _exception=desired_exception; }
};


/**
 * @brief Test the DataBlock class
 *
 * @details
 *
 */
class LmcServiceTest : public ::testing::Test
{
    protected:
        void SetUp() override;
        void TearDown() override;

        // beam resources methods
        grpc::Status configure_beam(bool dry_run = false);
        grpc::Status configure_beam(const ska::pst::lmc::ConfigureBeamRequest& request);
        grpc::Status get_beam_configuration(ska::pst::lmc::GetBeamConfigurationResponse* response);
        grpc::Status deconfigure_beam();

        // scan configuration methods
        grpc::Status configure_scan(bool dry_run = false);
        grpc::Status deconfigure_scan();
        grpc::Status get_scan_configuration(ska::pst::lmc::GetScanConfigurationResponse* response);

        // scan methods
        grpc::Status start_scan();
        grpc::Status stop_scan();

        // error handling methods
        grpc::Status abort();
        grpc::Status reset();
        grpc::Status restart();
        grpc::Status go_to_fault(const std::string& fault_message);

        // get environment
        grpc::Status get_env(ska::pst::lmc::GetEnvironmentResponse* response);

        grpc::Status get_state(ska::pst::lmc::GetStateResponse*);
        void assert_state(ska::pst::lmc::ObsState);

    public:
        LmcServiceTest();
        ~LmcServiceTest() = default;

        int _port = 0;
        std::shared_ptr<ska::pst::common::LmcService> _service{nullptr};
        std::shared_ptr<TestLmcServiceHandler> _handler{nullptr};
        std::shared_ptr<grpc::Channel> _channel{nullptr};
        std::shared_ptr<ska::pst::lmc::PstLmcService::Stub> _stub{nullptr};

};

} // namespace ska::pst::common::test

#endif // SKA_PST_SMRB_TESTS_LmcServiceTest_h
