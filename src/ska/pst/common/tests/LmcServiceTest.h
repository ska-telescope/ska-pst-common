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
#include "ska/pst/common/LmcService.h"
#include "ska/pst/common/LmcServiceHandler.h"
#include "ska/pst/lmc/ska_pst_lmc.grpc.pb.h"

#ifndef SKA_PST_SMRB_TESTS_LmcServiceTest_h
#define SKA_PST_SMRB_TESTS_LmcServiceTest_h

namespace ska {
namespace pst {
namespace common {
namespace test {

class TestLmcServiceHandler : public ska::pst::common::LmcServiceHandler {
    public:
        TestLmcServiceHandler() {
            ON_CALL(*this, assign_resources).WillByDefault([this](const ska::pst::lmc::ResourceConfiguration &new_resources) {
                resources.CopyFrom(new_resources);
                resources_assigned = true;
            });
            ON_CALL(*this, release_resources).WillByDefault([this]() {
                resources.Clear();
                resources_assigned = false;
            });
            ON_CALL(*this, get_assigned_resources).WillByDefault([this](ska::pst::lmc::ResourceConfiguration *response) {
                response->CopyFrom(resources);
            });

            ON_CALL(*this, configure).WillByDefault([this](const ska::pst::lmc::ScanConfiguration &configuration) {
                scan_configuration.CopyFrom(configuration);
                configured = true;
            });
            ON_CALL(*this, deconfigure).WillByDefault([this]() {
                scan_configuration.Clear();
                configured = false;
            });
            ON_CALL(*this, get_scan_configuration).WillByDefault([this](ska::pst::lmc::ScanConfiguration *configuration) {
                configuration->CopyFrom(scan_configuration);
            });


            ON_CALL(*this, scan).WillByDefault([this](const ska::pst::lmc::ScanRequest &request) {
                scanning = true;
            });
            ON_CALL(*this, end_scan).WillByDefault([this]() {
                scanning = false;
            });

            ON_CALL(*this, get_monitor_data).WillByDefault([this](ska::pst::lmc::MonitorData *data) {
                auto *test_monitor_data = data->mutable_test();
                auto *values = test_monitor_data->mutable_data();

                (*values)["hot"] = "cold";
            });
        }

        // testing fields
        bool resources_assigned{false};
        bool configured{false};
        bool scanning{false};
        ska::pst::lmc::ResourceConfiguration resources{};
        ska::pst::lmc::ScanConfiguration scan_configuration{};

        // Resources
        MOCK_METHOD(void, assign_resources, (const ska::pst::lmc::ResourceConfiguration &resources), (override));
        MOCK_METHOD(void, release_resources, (), (override));
        MOCK_METHOD(void, get_assigned_resources, (ska::pst::lmc::ResourceConfiguration *response), (override));
        bool are_resources_assigned() const noexcept override {
            return resources_assigned;
        }

        // Scan configuration
        MOCK_METHOD(void, configure, (const ska::pst::lmc::ScanConfiguration &configuration), (override));
        MOCK_METHOD(void, deconfigure, (), (override));
        MOCK_METHOD(void, get_scan_configuration, (ska::pst::lmc::ScanConfiguration *configuration), (override));
        bool is_configured() const noexcept override {
            return configured;
        }

        // Scan methods
        MOCK_METHOD(void, scan, (const ska::pst::lmc::ScanRequest &request), (override));
        MOCK_METHOD(void, end_scan, (), (override));
        bool is_scanning() const noexcept override {
            return scanning;
        }

        // Monitor
        MOCK_METHOD(void, get_monitor_data, (ska::pst::lmc::MonitorData *data), (override));
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
        grpc::Status assign_resources();
        grpc::Status assign_resources(ska::pst::lmc::AssignResourcesRequest request);
        grpc::Status get_assigned_resources(ska::pst::lmc::GetAssignedResourcesResponse* response);
        grpc::Status release_resources();

        // scan configuration methods
        grpc::Status configure();
        grpc::Status deconfigure();
        grpc::Status get_scan_configuration(ska::pst::lmc::GetScanConfigurationResponse* response);

        // scan methods
        grpc::Status scan();
        grpc::Status end_scan();

        grpc::Status abort();
        grpc::Status reset();
        grpc::Status restart();

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

} // namespace test
} // namespace common
} // namespace pst
} // namespace ska

#endif // SKA_PST_SMRB_TESTS_LmcServiceTest_h
