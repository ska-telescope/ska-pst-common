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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ska/pst/common/statemodel/ApplicationManager.h"

#ifndef SKA_PST_COMMON_TESTS_ApplicationManagerTest_h
#define SKA_PST_COMMON_TESTS_ApplicationManagerTest_h

namespace ska {
namespace pst {
namespace common {
namespace test {

/**
 * @brief Test the ApplicationManager class
 *
 * @details
 *
 */
class TestApplicationManager : public ska::pst::common::ApplicationManager
{
  public:
    TestApplicationManager() : ApplicationManager("TestApplicationManager") {
      ON_CALL(*this, _set_command).WillByDefault([this](Command cmd) {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::_set_command cmd=[{}]", get_name(cmd));
          command = cmd;
      });
      ON_CALL(*this, _set_state).WillByDefault([this](State required) {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::_set_state state=[{}] required=[{}]", get_name(state), get_name(required));
          state = required;
      });
      /*
      ON_CALL(*this, perform_initialise).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise");
          wait_for(Initialise);
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise state={} required_state={}",state_names[state] , state_names[Idle]);          
          set_state(Idle);
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise state={}",state_names[state]);
        });
      ON_CALL(*this, perform_terminate).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_terminate");
          wait_for(Terminate);
          set_state(Unknown);
        });
      */
      ON_CALL(*this, perform_configure_beam).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_configure_beam");
          wait_for(ConfigureBeam);
          set_state(BeamConfigured);
        });
      ON_CALL(*this, perform_configure_scan).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_configure_scan");
          wait_for(ConfigureScan);
          set_state(ScanConfigured);
        });
      ON_CALL(*this, perform_scan).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_scan");
          wait_for(StartScan);
          set_state(Scanning);
        });
      ON_CALL(*this, perform_stop_scan).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_stop_scan");
          wait_for(StopScan);
          set_state(ScanConfigured);
        });
      ON_CALL(*this, perform_deconfigure_scan).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_deconfigure_scan");
          wait_for(DeconfigureScan);
          set_state(BeamConfigured);
        });
      ON_CALL(*this, perform_deconfigure_beam).WillByDefault([this]() {
          spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_deconfigure_beam");
          wait_for(DeconfigureBeam);
          set_state(Idle);
        });
      // ON_CALL(*this, perform_reset).WillByDefault([this]() {
      //     spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_reset");
      //     wait_for(Reset);
      //     set_state(Idle);
      //   });
    }
    ~TestApplicationManager() = default;

    // Resources
    MOCK_METHOD(void, _set_command, (Command cmd));
    MOCK_METHOD(void, _set_state, (State required));
    // MOCK_METHOD(void, perform_initialise, (), (override));
    // MOCK_METHOD(void, perform_terminate, (), (override));
    MOCK_METHOD(void, perform_configure_beam, (), (override));
    MOCK_METHOD(void, perform_configure_scan, (), (override));
    MOCK_METHOD(void, perform_scan, (), (override));
    MOCK_METHOD(void, perform_stop_scan, (), (override));
    MOCK_METHOD(void, perform_deconfigure_scan, (), (override));
    MOCK_METHOD(void, perform_deconfigure_beam, (), (override));
    MOCK_METHOD(void, perform_reset, (), (override));

    void get_logs_state_and_command(std::shared_ptr<TestApplicationManager> _applicationmanager, std::string method_name);
    void perform_initialise();
    void perform_terminate();

  private:
};

class ApplicationManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override;
    void TearDown() override;
  public:
    ApplicationManagerTest();
    ~ApplicationManagerTest() = default;

    std::shared_ptr<TestApplicationManager> _applicationmanager{nullptr};

    // Resources
    ska::pst::common::AsciiHeader beam_config;
    ska::pst::common::AsciiHeader scan_config;
    ska::pst::common::AsciiHeader startscan_config;
  private:
};
}
}
}
}

#endif // SKA_PST_COMMON_TESTS_ApplicationManagerTest_h