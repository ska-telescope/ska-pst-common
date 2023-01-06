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
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise");
    wait_for(Initialise);
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise state={} required_state={}",state_names[state] , state_names[Idle]);          
    set_state(Idle);
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_initialise state={}",state_names[state]);
  }

  void TestApplicationManager::perform_terminate()
  {
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_terminate");
    wait_for(Terminate);
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_terminate state={} required_state={}",state_names[state] , state_names[Idle]);          
    set_state(Unknown);
    spdlog::trace("ska::pst::common::test::TestApplicationManager::perform_terminate state={}",state_names[state]);
  }

  ApplicationManagerTest::ApplicationManagerTest()
    : ::testing::Test()
  {
  }

  void ApplicationManagerTest::SetUp() 
  {
    spdlog::trace("ska::pst::common::test::ApplicationManagerTest::SetUp");
    // get_logs_state_and_command(_applicationmanager, "ska::pst::common::test::ApplicationManagerTest::SetUp");
    _applicationmanager = std::make_shared<TestApplicationManager>();
    // ASSERT_EQ(Unknown, _applicationmanager->get_state());
    // EXPECT_CALL(*_applicationmanager, _set_command(Initialise));
    // _applicationmanager->_set_command(Initialise);
    // EXPECT_CALL(*_applicationmanager, perform_initialise());
    // _applicationmanager->perform_initialise();
    // ASSERT_EQ(Idle, _applicationmanager->get_state());
  }
  void ApplicationManagerTest::TearDown()
  {
    spdlog::trace("ska::pst::common::test::ApplicationManagerTest::TearDown");
    // EXPECT_CALL(*_applicationmanager, _set_state(Idle));
    // _applicationmanager = std::make_shared<TestApplicationManager>();
    // ASSERT_EQ(Unknown, _applicationmanager->get_state());
    EXPECT_CALL(*_applicationmanager, _set_command(Terminate));
    _applicationmanager->_set_command(Terminate);
    // EXPECT_CALL(*_applicationmanager, perform_terminate());
    // _applicationmanager->perform_terminate();
    // ASSERT_EQ(Unknown, _applicationmanager->get_state());
    _applicationmanager = nullptr;
  }

  TEST_F(ApplicationManagerTest, test_happy_path) // NOLINT
  {
    std::string test_f;
    std::string test_msg;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_happy_path";

    spdlog::trace(test_f);

    // perform_configure_beam
    get_logs_state_and_command(_applicationmanager, ("{} perform_configure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(ConfigureBeam));
    _applicationmanager->_set_command(ConfigureBeam);
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->perform_configure_beam();
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    get_logs_state_and_command(_applicationmanager, ("{} perform_configure_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(ConfigureScan));
    _applicationmanager-> _set_command(ConfigureScan);
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->perform_configure_scan();
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    get_logs_state_and_command(_applicationmanager, ("{} perform_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(StartScan));
    _applicationmanager-> _set_command(StartScan);
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->perform_scan();
    ASSERT_EQ(Scanning, _applicationmanager->get_state());

    get_logs_state_and_command(_applicationmanager, ("{} perform_stop_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(StopScan));
    _applicationmanager-> _set_command(StopScan);
    EXPECT_CALL(*_applicationmanager, perform_stop_scan());
    _applicationmanager->perform_stop_scan();
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());
    
    get_logs_state_and_command(_applicationmanager, ("{} perform_deconfigure_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(DeconfigureScan));
    _applicationmanager-> _set_command(DeconfigureScan);
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_scan());
    _applicationmanager->perform_deconfigure_scan();
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    
    get_logs_state_and_command(_applicationmanager, ("{} perform_deconfigure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(DeconfigureBeam));
    _applicationmanager-> _set_command(DeconfigureBeam);
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_beam());
    _applicationmanager->perform_deconfigure_beam();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
  }

  TEST_F(ApplicationManagerTest, test_reset) // NOLINT
  {
    std::string test_f;
    std::string test_msg;
    test_f="ska::pst::common::test::ApplicationManagerTest::test_happy_path";

    spdlog::trace(test_f);

    // perform_configure_beam
    get_logs_state_and_command(_applicationmanager, ("{} perform_configure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(ConfigureBeam));
    _applicationmanager->_set_command(ConfigureBeam);
    EXPECT_CALL(*_applicationmanager, perform_configure_beam());
    _applicationmanager->perform_configure_beam();
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());

    get_logs_state_and_command(_applicationmanager, ("{} perform_configure_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(ConfigureScan));
    _applicationmanager-> _set_command(ConfigureScan);
    EXPECT_CALL(*_applicationmanager, perform_configure_scan());
    _applicationmanager->perform_configure_scan();
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());

    get_logs_state_and_command(_applicationmanager, ("{} perform_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(StartScan));
    _applicationmanager-> _set_command(StartScan);
    EXPECT_CALL(*_applicationmanager, perform_scan());
    _applicationmanager->perform_scan();
    ASSERT_EQ(Scanning, _applicationmanager->get_state());

    get_logs_state_and_command(_applicationmanager, ("{} perform_stop_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(StopScan));
    _applicationmanager-> _set_command(StopScan);
    EXPECT_CALL(*_applicationmanager, perform_stop_scan());
    _applicationmanager->perform_stop_scan();
    ASSERT_EQ(ScanConfigured, _applicationmanager->get_state());
    
    get_logs_state_and_command(_applicationmanager, ("{} perform_deconfigure_scan", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(DeconfigureScan));
    _applicationmanager-> _set_command(DeconfigureScan);
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_scan());
    _applicationmanager->perform_deconfigure_scan();
    ASSERT_EQ(BeamConfigured, _applicationmanager->get_state());
    
    get_logs_state_and_command(_applicationmanager, ("{} perform_deconfigure_beam", test_f));
    EXPECT_CALL(*_applicationmanager, _set_command(DeconfigureBeam));
    _applicationmanager-> _set_command(DeconfigureBeam);
    EXPECT_CALL(*_applicationmanager, perform_deconfigure_beam());
    _applicationmanager->perform_deconfigure_beam();
    ASSERT_EQ(Idle, _applicationmanager->get_state());
  }

} // test
} // common
} // pst
} // ska