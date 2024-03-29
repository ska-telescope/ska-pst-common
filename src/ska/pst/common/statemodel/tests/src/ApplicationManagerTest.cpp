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
#include "ska/pst/common/definitions.h"
#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/statemodel/tests/ApplicationManagerTest.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test
{

  void log_state_and_command(const std::shared_ptr<TestApplicationManager>& _applicationmanager, const std::string& method_name)
  {
    SPDLOG_TRACE("{} state={} command={}",method_name ,_applicationmanager->get_name(_applicationmanager->get_state()) ,_applicationmanager->get_name(_applicationmanager->get_command()));
  }

  void TestApplicationManager::perform_initialise()
  {
    SPDLOG_TRACE("ska::pst::common::test::TestApplicationManager::perform_initialise mock_function");
  }

  void TestApplicationManager::perform_terminate()
  {
    SPDLOG_TRACE("ska::pst::common::test::TestApplicationManager::perform_terminate mock_function");
  }

  void TestApplicationManager::validate_configure_beam(const ska::pst::common::AsciiHeader& beam_config, ska::pst::common::ValidationContext *context)
  {
    SPDLOG_TRACE("ska::pst::common::test::TestApplicationManager::validate_configure_beam");
    if (!beam_config.has("beam_config-FOO"))
    {
      context->add_validation_error<std::string>("beam_config-FOO", "<none>", "key not found");
    }
  }

  void TestApplicationManager::validate_configure_scan(const ska::pst::common::AsciiHeader& scan_config, ska::pst::common::ValidationContext *context)
  {
    SPDLOG_TRACE("ska::pst::common::test::TestApplicationManager::validate_configure_scan");
    if (!scan_config.has("scan_config-FOO"))
    {
      context->add_validation_error<std::string>("scan_config-FOO", "<none>", "key not found");
    }
  }

  void TestApplicationManager::validate_start_scan(const ska::pst::common::AsciiHeader& startscan_config)
  {
    SPDLOG_TRACE("ska::pst::common::test::TestApplicationManager::validate_start_scan");
    try
    {
      ASSERT_EQ(startscan_config.get_val("startscan_config-FOO"), "BAR");
    }
    catch (const std::exception& exc)
    {
      throw ska::pst::common::pst_validation_error("TestApplicationManager::validate_start_scan startscan_config[startscan_config-FOO] not found");
    }
  }

  ApplicationManagerTest::ApplicationManagerTest()
    : ::testing::Test()
  {
  }

  void ApplicationManagerTest::SetUp()
  {
    SPDLOG_TRACE("ska::pst::common::test::ApplicationManagerTest::SetUp");
    _applicationmanager = std::make_shared<TestApplicationManager>();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
  }

  void ApplicationManagerTest::TearDown()
  {
    SPDLOG_TRACE("ska::pst::common::test::ApplicationManagerTest::TearDown");
    beam_config.reset();
    scan_config.reset();
    startscan_config.reset();
    _applicationmanager->quit();
    _applicationmanager = nullptr;
  }

  TEST_F(ApplicationManagerTest, test_happy_path) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_happy_path";

    SPDLOG_TRACE(test_f);
    beam_config.set_val("beam_config-FOO", "BAR");
    scan_config.set_val("scan_config-FOO", "BAR");
    startscan_config.set_val("startscan_config-FOO", "BAR");

    EXPECT_FALSE(_applicationmanager->is_beam_configured());
    EXPECT_FALSE(_applicationmanager->is_scan_configured());
    EXPECT_FALSE(_applicationmanager->is_scanning());

    // perform_configure_beam
    log_state_and_command(_applicationmanager, test_f + " perform_configure_beam"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    ASSERT_TRUE(_applicationmanager->is_beam_configured());
    EXPECT_FALSE(_applicationmanager->is_scan_configured());
    EXPECT_FALSE(_applicationmanager->is_scanning());

    // perform_configure_scan
    log_state_and_command(_applicationmanager, test_f +" perform_configure_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->configure_scan(scan_config);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());
    EXPECT_TRUE(_applicationmanager->is_beam_configured());
    ASSERT_TRUE(_applicationmanager->is_scan_configured());
    EXPECT_FALSE(_applicationmanager->is_scanning());

    // perform_scan
    log_state_and_command(_applicationmanager, test_f +" perform_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_start_scan());
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->start_scan(startscan_config);
    ASSERT_EQ(Scanning, _applicationmanager->get_state());
    EXPECT_TRUE(_applicationmanager->is_beam_configured());
    EXPECT_TRUE(_applicationmanager->is_scan_configured());
    ASSERT_TRUE(_applicationmanager->is_scanning());

    // perform_stop_scan
    log_state_and_command(_applicationmanager, test_f +" perform_stop_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_stop_scan());
    _applicationmanager->stop_scan();
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());
    EXPECT_TRUE(_applicationmanager->is_beam_configured());
    EXPECT_TRUE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());

    // perform_deconfigure_scan
    log_state_and_command(_applicationmanager, test_f +" perform_deconfigure_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_scan());
    _applicationmanager->deconfigure_scan();
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    EXPECT_TRUE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    EXPECT_FALSE(_applicationmanager->is_scanning());

    // perform_deconfigure_beam
    log_state_and_command(_applicationmanager, test_f +" perform_deconfigure_beam"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_beam());
    _applicationmanager->deconfigure_beam();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    EXPECT_FALSE(_applicationmanager->is_scan_configured());
    EXPECT_FALSE(_applicationmanager->is_scanning());
  }

  TEST_F(ApplicationManagerTest, test_config_validations) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_config_validations";

    SPDLOG_TRACE(test_f);

    // validate_configure_beam
    log_state_and_command(_applicationmanager, test_f +" validate_configure_beam"); // NOLINT
    ASSERT_THROW(_applicationmanager->configure_beam(beam_config), ska::pst::common::pst_validation_error); // NOLINT
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_TRUE(_applicationmanager->is_idle());

    // Proceed to BeamConfigured
    beam_config.set_val("beam_config-FOO", "BAR");
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // validate_configure_scan
    log_state_and_command(_applicationmanager, test_f +" validate_configure_scan"); // NOLINT
    ASSERT_THROW(_applicationmanager->configure_scan(scan_config), ska::pst::common::pst_validation_error); // NOLINT
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // Proceed to ScanConfigured
    scan_config.set_val("scan_config-FOO", "BAR");
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->configure_scan(scan_config);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    // validate_configure_startscan
    log_state_and_command(_applicationmanager, test_f +" validate_configure_startscan"); // NOLINT
    ASSERT_THROW(_applicationmanager->start_scan(scan_config),ska::pst::common::pst_validation_error); // NOLINT
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

  TEST_F(ApplicationManagerTest, test_reset) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_reset";

    SPDLOG_TRACE(test_f);
    beam_config.set_val("beam_config-FOO", "BAR");
    scan_config.set_val("scan_config-FOO", "BAR");
    startscan_config.set_val("startscan_config-FOO", "BAR");

    // Trigger error in perform_configure_beam
    _applicationmanager->force_error=true;
    log_state_and_command(_applicationmanager, test_f +" configure_beam"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    ASSERT_THROW(_applicationmanager->perform_configure_beam(),std::runtime_error); // NOLINT

    log_state_and_command(_applicationmanager, test_f +" reset"); // NOLINT
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());

    // Proceed to BeamConfigured
    EXPECT_CALL(*_applicationmanager, _set_state(BeamConfigured));
    _applicationmanager->_set_state(BeamConfigured);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // Trigger error in perform_configure_scan
    _applicationmanager->force_error=true;
    log_state_and_command(_applicationmanager, test_f +" configure_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    ASSERT_THROW(_applicationmanager->perform_configure_scan(),std::runtime_error); // NOLINT

    log_state_and_command(_applicationmanager, test_f +" reset"); // NOLINT
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_previous_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());

    // Proceed to ScanConfigured
    EXPECT_CALL(*_applicationmanager, _set_state(ScanConfigured));
    _applicationmanager->_set_state(ScanConfigured);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());


    // Trigger error in perform_start_scan
    _applicationmanager->force_error=true;
    log_state_and_command(_applicationmanager, test_f +" start_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_start_scan());
    ASSERT_THROW(_applicationmanager->perform_start_scan(),std::runtime_error); // NOLINT

    log_state_and_command(_applicationmanager, test_f +" reset"); // NOLINT
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_previous_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());

    // Proceed to Scanning
    EXPECT_CALL(*_applicationmanager, _set_state(Scanning));
    _applicationmanager->_set_state(Scanning);
    ASSERT_EQ(Scanning, _applicationmanager->get_state());

    // Trigger error in perform_stop_scan
    _applicationmanager->force_error=true;
    log_state_and_command(_applicationmanager, test_f +" stop_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_stop_scan());
    ASSERT_THROW(_applicationmanager->perform_stop_scan(),std::runtime_error); // NOLINT

    log_state_and_command(_applicationmanager, test_f +" reset"); // NOLINT
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());

    // Proceed to ScanConfigured
    EXPECT_CALL(*_applicationmanager, _set_state(ScanConfigured));
    _applicationmanager->_set_state(ScanConfigured);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());


    // Trigger error in perform_deconfigure_scan
    _applicationmanager->force_error=true;
    log_state_and_command(_applicationmanager, test_f +" deconfigure_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_scan());
    ASSERT_THROW(_applicationmanager->perform_deconfigure_scan(),std::runtime_error); // NOLINT

    log_state_and_command(_applicationmanager, test_f +" reset"); // NOLINT
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());

    // Proceed to BeamConfigured
    EXPECT_CALL(*_applicationmanager, _set_state(BeamConfigured));
    _applicationmanager->_set_state(BeamConfigured);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    // Trigger error in perform_deconfigure_beam
    _applicationmanager->force_error=true;
    log_state_and_command(_applicationmanager, test_f +" deconfigure_beam"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_beam());
    ASSERT_THROW(_applicationmanager->perform_deconfigure_beam(),std::runtime_error); // NOLINT

    log_state_and_command(_applicationmanager, test_f +" deconfigure_beam error encountered"); // NOLINT
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());
    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());
  }

  TEST_F(ApplicationManagerTest, test_set_config) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_set_config";

    SPDLOG_TRACE(test_f);
    beam_config.set_val("beam_config-FOO", "BAR");
    scan_config.set_val("scan_config-FOO", "BAR");
    startscan_config.set_val("startscan_config-FOO", "BAR");

    // perform_configure_beam
    log_state_and_command(_applicationmanager, test_f + " perform_configure_beam"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    ska::pst::common::AsciiHeader new_beam_config;
    new_beam_config = _applicationmanager->get_beam_configuration();
    ASSERT_EQ(beam_config.get_val("beam_config-FOO"), new_beam_config.get_val("beam_config-FOO"));

    // perform_configure_scan
    log_state_and_command(_applicationmanager, test_f +" perform_configure_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->configure_scan(scan_config);
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());
    ska::pst::common::AsciiHeader new_scan_config;
    new_scan_config = _applicationmanager->get_scan_configuration();
    ASSERT_EQ(scan_config.get_val("scan_config-FOO"), new_scan_config.get_val("scan_config-FOO"));

    // perform_scan
    log_state_and_command(_applicationmanager, test_f +" perform_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_start_scan());
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->start_scan(startscan_config);
    ASSERT_EQ(Scanning, _applicationmanager->get_state());
    ska::pst::common::AsciiHeader new_startscan_config;
    new_startscan_config = _applicationmanager->get_startscan_configuration();
    ASSERT_EQ(startscan_config.get_val("startscan_config-FOO"), new_startscan_config.get_val("startscan_config-FOO"));

  }

  TEST_F(ApplicationManagerTest, test_exception_during_perform_scan) // NOLINT
  {
    std::string test_f;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_reset";

    SPDLOG_TRACE(test_f);
    beam_config.set_val("beam_config-FOO", "BAR");
    scan_config.set_val("scan_config-FOO", "BAR");
    startscan_config.set_val("startscan_config-FOO", "BAR");

    log_state_and_command(_applicationmanager, test_f +" configure_beam"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->configure_beam(beam_config);


    log_state_and_command(_applicationmanager, test_f +" configure_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->configure_scan(scan_config);

    // Trigger error in perform_scan
    _applicationmanager->force_scan_error=true;

    log_state_and_command(_applicationmanager, test_f +" start_scan"); // NOLINT
    EXPECT_CALL(*_applicationmanager, perform_start_scan());
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->start_scan(startscan_config);

    usleep(ska::pst::common::microseconds_per_decisecond);

    // ensure that state has been set to RuntimeError
    ASSERT_EQ(RuntimeError, _applicationmanager->get_state());

    // additionally assert that perform_scan does throw a runtime_error
    EXPECT_CALL(*_applicationmanager, perform_scan());
    ASSERT_THROW(_applicationmanager->perform_scan(),std::runtime_error); // NOLINT

    EXPECT_CALL(*_applicationmanager, perform_reset());
    _applicationmanager->reset();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
    ASSERT_FALSE(_applicationmanager->is_beam_configured());
    ASSERT_FALSE(_applicationmanager->is_scan_configured());
    ASSERT_FALSE(_applicationmanager->is_scanning());
  }

} // namespace ska::pst::common::test