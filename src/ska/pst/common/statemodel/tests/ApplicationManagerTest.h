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
      // For initialise
      ON_CALL(*this, _wait_for_state).WillByDefault([this](State required) {
          wait_for_state(required);
      });
      // For jumping states
      ON_CALL(*this, _set_state).WillByDefault([this](State required) {
          set_state(required);
      });

      ON_CALL(*this, perform_configure_beam).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_configure_beam force_error=true command={}");
          }
        });
      ON_CALL(*this, perform_configure_scan).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_configure_scan force_error=true");
          }
        });
      ON_CALL(*this, perform_scan).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_scan force_error=true");
          }
        });
      ON_CALL(*this, perform_start_scan).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_start_scan force_error=true");
          }
        });
      ON_CALL(*this, perform_stop_scan).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_stop_scan force_error=true");
          }
        });
      ON_CALL(*this, perform_deconfigure_scan).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_deconfigure_scan force_error=true");
          }
        });
      ON_CALL(*this, perform_deconfigure_beam).WillByDefault([this]() {
          if(force_error)
          {
            // Mock RuntimeError state
            set_state(RuntimeError);
            throw std::runtime_error("ska::pst::common::test::TestApplicationManager::perform_deconfigure_beam force_error=true");
          }
        });
      ON_CALL(*this, perform_reset).WillByDefault([this]() {
          force_error=false;
        });
    }
    ~TestApplicationManager() = default;

    // Resources
    MOCK_METHOD(void, _wait_for_state, (State required));
    MOCK_METHOD(void, _set_state, (State required));
    MOCK_METHOD(void, perform_configure_beam, (), (override));
    MOCK_METHOD(void, perform_configure_scan, (), (override));
    MOCK_METHOD(void, perform_scan, (), (override));
    MOCK_METHOD(void, perform_start_scan, (), (override));
    MOCK_METHOD(void, perform_stop_scan, (), (override));
    MOCK_METHOD(void, perform_deconfigure_scan, (), (override));
    MOCK_METHOD(void, perform_deconfigure_beam, (), (override));
    MOCK_METHOD(void, perform_reset, (), (override));

    void logs_state_and_command(std::shared_ptr<TestApplicationManager> _applicationmanager, std::string method_name);
    void perform_initialise();
    void perform_terminate();
    void validate_configure_beam(const ska::pst::common::AsciiHeader& config);
    void validate_configure_scan(const ska::pst::common::AsciiHeader& config);
    void validate_start_scan(const ska::pst::common::AsciiHeader& config);

    bool force_error=false;
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
    ska::pst::common::AsciiHeader beam_config{};
    ska::pst::common::AsciiHeader scan_config{};
    ska::pst::common::AsciiHeader startscan_config{};

  private:
};
}
}
}
}

#endif // SKA_PST_COMMON_TESTS_ApplicationManagerTest_h