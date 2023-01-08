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
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/utils/Timer.h"
#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/statemodel/tests/ApplicationManagerTest.h"

auto main(int argc, char* argv[]) -> int
{
  spdlog::set_level(spdlog::level::trace);
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska {
namespace pst {
namespace common {
namespace test {

  void get_logs_state_and_command(std::shared_ptr<TestApplicationManager> _applicationmanager, std::string method_name)
  {
    spdlog::trace("{} state={} command={}",method_name ,_applicationmanager->get_name(_applicationmanager->get_state()),_applicationmanager->get_name(_applicationmanager->get_command()));
  }

  void TestApplicationManager::perform_initialise()
  {
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise mock_function");
  }
  void TestApplicationManager::perform_terminate()
  {
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_terminate mock_function");
  }

  void TestApplicationManager::validate_configure_beam(const ska::pst::common::AsciiHeader& beam_config) 
  {
    spdlog::trace("ska::pst::common::test::TestApplicationManager::validate_configure_beam");
    try
    {
      beam_config.get_val("beam_config-FOO");
    }
    catch (const std::exception& exc)
    {
      throw std::runtime_error("TestApplicationManager::validate_configure_beam beam_config[beam_config-FOO] not found");
    }
  }
  void TestApplicationManager::validate_configure_scan(const ska::pst::common::AsciiHeader& scan_config) 
  {
    spdlog::trace("ska::pst::common::test::TestApplicationManager::validate_configure_scan");
    try
    {
      scan_config.get_val("scan_config-FOO");
    }
    catch (const std::exception& exc)
    {
      throw std::runtime_error("TestApplicationManager::validate_configure_scan scan_config[scan_config-FOO] not found");
    }
  }

  void TestApplicationManager::validate_start_scan(const ska::pst::common::AsciiHeader& startscan_config) 
  {
    spdlog::trace("ska::pst::common::test::TestApplicationManager::validate_start_scan");
    try
    {
      startscan_config.get_val("startscan_config-FOO");
    }
    catch (const std::exception& exc)
    {
      throw std::runtime_error("TestApplicationManager::validate_start_scan startscan_config[startscan_config-FOO] not found");
    }
  }

  ApplicationManagerTest::ApplicationManagerTest()
    : ::testing::Test()
  {
  }

  void ApplicationManagerTest::SetUp() 
  {
    spdlog::trace("ska::pst::common::test::ApplicationManagerTest::SetUp");
    _applicationmanager = std::make_shared<TestApplicationManager>();
    // Runtime error
    // beam_config.load_from_file(test_data_file("beam_config.txt"));
    // scan_config.load_from_file(test_data_file("scan_config.txt"));
    // startscan_config.load_from_file(test_data_file("startscan_config.txt"));

    EXPECT_CALL(*_applicationmanager, _wait_for(Idle));
    _applicationmanager->_wait_for(Idle);
    ASSERT_EQ(Idle, _applicationmanager->get_state());
  }
  void ApplicationManagerTest::TearDown()
  {
    spdlog::trace("ska::pst::common::test::ApplicationManagerTest::TearDown");
    _applicationmanager->quit();
    _applicationmanager = nullptr;
  }

  TEST_F(ApplicationManagerTest, test_happy_path) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_happy_path";

    spdlog::trace(test_f);
    beam_config.set_val("beam_config-FOO", "BAR");
    scan_config.set_val("scan_config-FOO", "BAR");
    startscan_config.set_val("startscan_config-FOO", "BAR");

    // perform_configure_beam
    get_logs_state_and_command(_applicationmanager, ("{} perform_configure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    
    // perform_configure_scan
    get_logs_state_and_command(_applicationmanager, ("{} perform_configure_scan", test_f));
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->configure_scan(scan_config);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    // perform_scan
    get_logs_state_and_command(_applicationmanager, ("{} perform_scan", test_f));
    EXPECT_CALL(*_applicationmanager, perform_start_scan());
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->start_scan(startscan_config);
    ASSERT_EQ(Scanning, _applicationmanager->get_state());

    // perform_stop_scan
    get_logs_state_and_command(_applicationmanager, ("{} perform_stop_scan", test_f));
    EXPECT_CALL(*_applicationmanager, perform_stop_scan());
    _applicationmanager->stop_scan();
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    // perform_deconfigure_scan
    get_logs_state_and_command(_applicationmanager, ("{} perform_deconfigure_scan", test_f));
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_scan());
    _applicationmanager->deconfigure_scan();
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // perform_deconfigure_beam
    get_logs_state_and_command(_applicationmanager, ("{} perform_deconfigure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_beam());
    _applicationmanager->deconfigure_beam();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
  }

  TEST_F(ApplicationManagerTest, test_config_validations) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_config_validations";

    spdlog::trace(test_f);

    // validate_configure_beam
    get_logs_state_and_command(_applicationmanager, ("{} validate_configure_beam", test_f));
    ASSERT_THROW(_applicationmanager->configure_beam(beam_config),std::runtime_error);
    ASSERT_EQ(Idle, _applicationmanager->get_state());

    // Proceed to BeamConfigured
    beam_config.set_val("beam_config-FOO", "BAR");
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // validate_configure_scan
    get_logs_state_and_command(_applicationmanager, ("{} validate_configure_scan", test_f));
    ASSERT_THROW(_applicationmanager->configure_scan(scan_config),std::runtime_error);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // Proceed to ScanConfigured
    scan_config.set_val("scan_config-FOO", "BAR");
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->configure_scan(scan_config);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    // validate_configure_startscan
    get_logs_state_and_command(_applicationmanager, ("{} validate_configure_startscan", test_f));
    ASSERT_THROW(_applicationmanager->start_scan(scan_config),std::runtime_error);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    // Proceed to Scanning
    startscan_config.set_val("startscan_config-FOO", "BAR");
    EXPECT_CALL(*_applicationmanager, perform_start_scan());
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->start_scan(startscan_config);
    ASSERT_EQ(Scanning, _applicationmanager->get_state());

    // Cleanup
    EXPECT_CALL(*_applicationmanager, perform_stop_scan());
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_scan());
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_beam());
  }

  TEST_F(ApplicationManagerTest, test_reset) // NO_LINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_reset";

    spdlog::trace(test_f);
    beam_config.set_val("beam_config-FOO", "BAR");
    scan_config.set_val("scan_config-FOO", "BAR");
    startscan_config.set_val("startscan_config-FOO", "BAR");

    /* 
    // Trigger error in perform_configure_beam
    _applicationmanager->force_error=true;
    get_logs_state_and_command(_applicationmanager, ("{} configure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    ASSERT_THROW(_applicationmanager->configure_beam(beam_config),std::runtime_error);
    // ensure that state is runtime error
    _applicationmanager->_wait_for(RuntimeError);
  
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());

    // Proceed to BeamConfigured
    _applicationmanager->force_error=false;
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    */

    // Trigger error in perform_configure_scan
    // Trigger error in perform_start_scan
    // Trigger error in perform_scan
    // Trigger error in perform_stop_scan
    // Trigger error in perform_stop_scan
    // Trigger error in perform_deconfigure_scan
    // Trigger error in perform_deconfigure_beam

  }
} // test
} // common
} // pst
} // ska