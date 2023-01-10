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

#include "ska/pst/common/statemodel/StateModelException.h"
#include "ska/pst/common/statemodel/StateModel.h"

#ifndef SKA_PST_COMMON_TESTS_StateModelTest_h
#define SKA_PST_COMMON_TESTS_StateModelTest_h

namespace ska {
namespace pst {
namespace common {
namespace test {

/**
 * @brief Test the StateModel class
 *
 * @details
 *
 */
class TestStateModel : public StateModel
{
  public:
    TestStateModel() {
      ON_CALL(*this, set_command).WillByDefault([this](Command cmd) {
          command = cmd;
      });
      ON_CALL(*this, set_state).WillByDefault([this](State required) {
          state = required;
      });
    }
    ~TestStateModel() = default;

    void validate_configure_beam(const ska::pst::common::AsciiHeader& config) {; };
    void validate_configure_scan(const ska::pst::common::AsciiHeader& config) {; };
    void validate_start_scan(const ska::pst::common::AsciiHeader& config) {; };

    // Resources
    MOCK_METHOD(void, set_command, (Command cmd));
    MOCK_METHOD(void, set_state, (State required));
};

class StateModelTest : public ::testing::Test
{
  protected:
    void SetUp() override;
    void TearDown() override;
  public:
    StateModelTest();
    ~StateModelTest() = default;

    void assert_command(Command cmd);
    void assert_set_command(Command cmd);
    void _set_state(State state);

    ska::pst::common::AsciiHeader beam_config;
    ska::pst::common::AsciiHeader scan_config;
    ska::pst::common::AsciiHeader startscan_config;

    std::shared_ptr<TestStateModel> _statemodel{nullptr};
  private:
};
} // test
} // common
} // pst
} // ska
#endif // SKA_PST_COMMON_StateModelTest_h