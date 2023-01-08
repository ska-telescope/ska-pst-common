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

#include "ska/pst/common/statemodel/tests/StateModelTest.h"

auto main(int argc, char* argv[]) -> int
{
  spdlog::set_level(spdlog::level::trace);
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska {
namespace pst {
namespace common {
namespace test {

StateModelTest::StateModelTest()
: ::testing::Test()
{
}

void StateModelTest::SetUp()
{
    spdlog::trace("StateModelTest::SetUp()");
    spdlog::trace("StateModelTest::SetUp() Load configurations");
    // beam_config.load_from_file(test_data_file("beam_config.txt"));
    // scan_config.load_from_file(test_data_file("scan_config.txt"));

    _statemodel = std::make_shared<TestStateModel>();
}


void StateModelTest::TearDown()
{
}

void StateModelTest::assert_command(Command cmd)
{
    ASSERT_EQ(_statemodel->get_command(), cmd);
}

void StateModelTest::assert_set_command(Command cmd)
{
    EXPECT_CALL(*_statemodel,set_command(cmd));
    _statemodel->set_command(cmd);
    assert_command(cmd);
}

void StateModelTest::_set_state(State state)
{
    EXPECT_CALL(*_statemodel,set_state(state));
    _statemodel->set_state(state);
}

TEST_F(StateModelTest, test_construct_delete) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_construct_delete");
}

TEST_F(StateModelTest, test_set_command) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_set_command");
    assert_set_command(ConfigureBeam);
    assert_set_command(ConfigureScan);
    assert_set_command(StartScan);
    assert_set_command(StopScan);
    assert_set_command(DeconfigureScan);
    assert_set_command(DeconfigureBeam);
    assert_set_command(Reset);
    assert_set_command(Terminate);
}

TEST_F(StateModelTest, test_configure_beam) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_configure_beam");
    _set_state(Idle);
    // _statemodel->configure_beam(beam_config); // blocking call
    // assert_command(ConfigureBeam);

    // Test error ConfigureBeam on BeamConfigured
    _set_state(BeamConfigured);
    ASSERT_THROW(_statemodel->configure_beam(beam_config),std::runtime_error);
    ASSERT_NE(ConfigureBeam, _statemodel->get_command());

    // Test error ConfigureBeam on ScanConfigured
    _set_state(ScanConfigured);
    ASSERT_THROW(_statemodel->configure_beam(beam_config),std::runtime_error);
    ASSERT_NE(ConfigureBeam, _statemodel->get_command());

    // Test error ConfigureBeam on Scanning
    _set_state(Scanning);
    ASSERT_THROW(_statemodel->configure_beam(beam_config),std::runtime_error);
    ASSERT_NE(ConfigureBeam, _statemodel->get_command());

    // Test error ConfigureBeam on RuntimeError
    _set_state(RuntimeError);
    ASSERT_THROW(_statemodel->configure_beam(beam_config),std::runtime_error);
    ASSERT_NE(ConfigureBeam, _statemodel->get_command());

}

TEST_F(StateModelTest, test_configure_scan) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_configure_scan");
    EXPECT_CALL(*_statemodel,set_state(BeamConfigured));
    _statemodel->set_state(BeamConfigured);
    // _statemodel->configure_scan(scan_config); // blocking call
    // assert_command(ConfigureScan);

    // Test error ConfigureScan on Idle
    _set_state(Idle);
    ASSERT_THROW(_statemodel->configure_scan(scan_config),std::runtime_error);
    ASSERT_NE(ConfigureScan, _statemodel->get_command());

    // Test error ConfigureScan on ScanConfigured
    _set_state(ScanConfigured);
    ASSERT_THROW(_statemodel->configure_scan(scan_config),std::runtime_error);
    ASSERT_NE(ConfigureScan, _statemodel->get_command());

    // Test error ConfigureScan on Scanning
    _set_state(Scanning);
    ASSERT_THROW(_statemodel->configure_scan(scan_config),std::runtime_error);
    ASSERT_NE(ConfigureBeam, _statemodel->get_command());

    // Test error ConfigureScan on RuntimeError
    _set_state(RuntimeError);
    ASSERT_THROW(_statemodel->configure_scan(scan_config),std::runtime_error);
    ASSERT_NE(ConfigureBeam, _statemodel->get_command());
}

TEST_F(StateModelTest, test_start_scan) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_start_scan");
    EXPECT_CALL(*_statemodel,set_state(ScanConfigured));
    _statemodel->set_state(ScanConfigured);
    // _statemodel->start_scan(startscan_config); // blocking call
    // assert_command(StartScan);
    // Test error StartScan on Idle
    _set_state(Idle);
    ASSERT_THROW(_statemodel->start_scan(startscan_config),std::runtime_error);
    ASSERT_NE(StartScan, _statemodel->get_command());

    // Test error StartScan on BeamConfigured
    _set_state(BeamConfigured);
    ASSERT_THROW(_statemodel->start_scan(startscan_config),std::runtime_error);
    ASSERT_NE(StartScan, _statemodel->get_command());

    // Test error StartScan on Scanning
    _set_state(Scanning);
    ASSERT_THROW(_statemodel->start_scan(startscan_config),std::runtime_error);
    ASSERT_NE(StartScan, _statemodel->get_command());

    // Test error StartScan on RuntimeError
    _set_state(RuntimeError);
    ASSERT_THROW(_statemodel->start_scan(startscan_config),std::runtime_error);
    ASSERT_NE(StartScan, _statemodel->get_command());

}

TEST_F(StateModelTest, test_stop_scan) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_stop_scan");
    EXPECT_CALL(*_statemodel,set_state(Scanning));
    _statemodel->set_state(Scanning);
    // _statemodel->stop_scan(); // blocking call
    // assert_command(StopScan);
    // Test error StopScan on Idle
    _set_state(Idle);
    ASSERT_THROW(_statemodel->stop_scan(),std::runtime_error);
    ASSERT_NE(StopScan, _statemodel->get_command());

    // Test error StopScan on BeamConfigured
    _set_state(BeamConfigured);
    ASSERT_THROW(_statemodel->stop_scan(),std::runtime_error);
    ASSERT_NE(StopScan, _statemodel->get_command());

    // Test error StopScan on ScanConfigured
    _set_state(ScanConfigured);
    ASSERT_THROW(_statemodel->stop_scan(),std::runtime_error);
    ASSERT_NE(StopScan, _statemodel->get_command());

    // Test error StopScan on RuntimeError
    _set_state(RuntimeError);
    ASSERT_THROW(_statemodel->stop_scan(),std::runtime_error);
    ASSERT_NE(StopScan, _statemodel->get_command());

}

TEST_F(StateModelTest, test_deconfigure_scan) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_deconfigure_scan");
    EXPECT_CALL(*_statemodel,set_state(ScanConfigured));
    _statemodel->set_state(ScanConfigured);
    // _statemodel->deconfigure_scan(); // blocking call
    // assert_command(DeconfigureScan);
    // Test error DeconfigureScan on Idle
    _set_state(Idle);
    ASSERT_THROW(_statemodel->deconfigure_scan(),std::runtime_error);
    ASSERT_NE(DeconfigureScan, _statemodel->get_command());

    // Test error DeconfigureScan on BeamConfigured
    _set_state(BeamConfigured);
    ASSERT_THROW(_statemodel->deconfigure_scan(),std::runtime_error);
    ASSERT_NE(DeconfigureScan, _statemodel->get_command());

    // Test error DeconfigureScan on Scanning
    _set_state(Scanning);
    ASSERT_THROW(_statemodel->deconfigure_scan(),std::runtime_error);
    ASSERT_NE(DeconfigureScan, _statemodel->get_command());

    // Test error DeconfigureScan on RuntimeError
    _set_state(RuntimeError);
    ASSERT_THROW(_statemodel->deconfigure_scan(),std::runtime_error);
    ASSERT_NE(DeconfigureScan, _statemodel->get_command());

}

TEST_F(StateModelTest, test_deconfigure_beam) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_deconfigure_beam");
    EXPECT_CALL(*_statemodel,set_state(BeamConfigured));
    _statemodel->set_state(BeamConfigured);
    // _statemodel->deconfigure_beam(); // blocking call
    _set_state(Idle);
    ASSERT_THROW(_statemodel->deconfigure_beam(),std::runtime_error);
    ASSERT_NE(DeconfigureBeam, _statemodel->get_command());

    // Test error DeconfigureBeam on ScanConfigured
    _set_state(ScanConfigured);
    ASSERT_THROW(_statemodel->deconfigure_beam(),std::runtime_error);
    ASSERT_NE(DeconfigureBeam, _statemodel->get_command());

    // Test error DeconfigureBeam on Scanning
    _set_state(Scanning);
    ASSERT_THROW(_statemodel->deconfigure_beam(),std::runtime_error);
    ASSERT_NE(DeconfigureBeam, _statemodel->get_command());

    // Test error DeconfigureBeam on RuntimeError
    _set_state(RuntimeError);
    ASSERT_THROW(_statemodel->deconfigure_beam(),std::runtime_error);
    ASSERT_NE(DeconfigureBeam, _statemodel->get_command());

}

TEST_F(StateModelTest, test_reset) // NOLINT
{
    spdlog::trace("ska::pst::common::test::StateModelTest::test_reset");
    EXPECT_CALL(*_statemodel,set_state(RuntimeError));
    _statemodel->set_state(RuntimeError);
    // _statemodel->reset(); // blocking call

    // Test error reset on Idle
    _set_state(Idle);
    ASSERT_THROW(_statemodel->reset(),std::runtime_error);
    ASSERT_NE(Reset, _statemodel->get_command());

    // Test error reset on BeamConfigured
    _set_state(BeamConfigured);
    ASSERT_THROW(_statemodel->reset(),std::runtime_error);
    ASSERT_NE(Reset, _statemodel->get_command());

    // Test error reset on ScanConfigured
    _set_state(ScanConfigured);
    ASSERT_THROW(_statemodel->reset(),std::runtime_error);
    ASSERT_NE(Reset, _statemodel->get_command());

    // Test error reset on Scanning
    _set_state(Scanning);
    ASSERT_THROW(_statemodel->reset(),std::runtime_error);
    ASSERT_NE(Reset, _statemodel->get_command());



}

} // namespace test
} // namespace common
} // namespace pst
} // namespace ska
