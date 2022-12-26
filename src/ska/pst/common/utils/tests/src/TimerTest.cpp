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

#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/tests/TimerTest.h"
#include "ska/pst/common/utils/Timer.h"
#include "ska/pst/common/testutils/GtestMain.h"

auto main(int argc, char* argv[]) -> int
{
  spdlog::set_level(spdlog::level::debug);
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska {
namespace pst {
namespace common {
namespace test {

TimerTest::TimerTest()
    : ::testing::Test()
{
}

void TimerTest::SetUp()
{
}

void TimerTest::TearDown()
{
}

TEST_F(TimerTest, default_constructor) // NOLINT
{
  Timer epoch;
}

TEST_F(TimerTest, test_get_elapsed_microseconds) // NOLINT
{
  Timer timer;
  EXPECT_LE(timer.get_elapsed_microseconds(), 1000);
}

TEST_F(TimerTest, test_wait_until) // NOLINT
{
  Timer timer;
  static constexpr double delay_us = 100000;
  timer.wait_until(delay_us);
  double elapsed = timer.get_elapsed_microseconds();
  EXPECT_GE(elapsed, delay_us);
  EXPECT_LE(elapsed, delay_us * 2.0);
}

TEST_F(TimerTest, get_print_rates) // NOLINT
{
  Timer timer;
  static constexpr double delay_us = 1000;
  timer.wait_until(delay_us);
  static constexpr uint64_t nbytes = 8192;
  timer.print_rates(nbytes);
}

} // namepsace test
} // namepsace common
} // namepsace pst
} // namepsace ska