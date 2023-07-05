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

#include "ska/pst/common/definitions.h"
#include "ska/pst/common/utils/tests/TimeTest.h"
#include "ska/pst/common/utils/Time.h"
#include "ska/pst/common/testutils/GtestMain.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

TimeTest::TimeTest()
    : ::testing::Test()
{
}

void TimeTest::SetUp()
{
}

void TimeTest::TearDown()
{
}

TEST_F(TimeTest, default_constructor) // NOLINT
{
  Time epoch;
  EXPECT_EQ(epoch.get_gmtime(), "1970-01-01-00:00:00");
}

TEST_F(TimeTest, string_constructor) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_gmtime(), "2000-01-01-00:00:00");
}

TEST_F(TimeTest, set_time) // NOLINT
{
  Time epoch;
  epoch.set_time("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_gmtime(), "2000-01-01-00:00:00");
}

TEST_F(TimeTest, get_time) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_time(), 946684800);
}

TEST_F(TimeTest, get_gm_year) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_gm_year(), 2000);
}

TEST_F(TimeTest, get_gm_month) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_gm_month(), 0);
}

TEST_F(TimeTest, get_mjd_day) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_mjd_day(), 51544);
}

TEST_F(TimeTest, mjd2utctm) // NOLINT
{
  static constexpr double base_mjd = 51544;
  static constexpr time_t base_epoch = 946684800;
  static constexpr double seconds_per_day  = 86400;
  static constexpr double milliseconds_per_day = seconds_per_day * ska::pst::common::milliseconds_per_second;
  static constexpr unsigned seconds_to_test = 10;

  auto ntests = unsigned(seconds_to_test * milliseconds_per_second);
  for (unsigned i=0; i<ntests; i++)
  {
    double fractional_day = double(i) / milliseconds_per_day;
    double fractional_seconds = double(i) / ska::pst::common::milliseconds_per_second;
    time_t epoch = Time::mjd2utctm(base_mjd + fractional_day);
    time_t expected_epoch = base_epoch + int(floor(fractional_seconds));
    if (i % milliseconds_per_second >= (milliseconds_per_second/2))
    {
      expected_epoch++;
    }
    EXPECT_EQ(epoch, expected_epoch);
  }
}

TEST_F(TimeTest, get_fractional_time) // NOLINT
{
  Time epoch("2000-01-01-00:00:00.123");
  EXPECT_EQ(epoch.get_fractional_time(), 0.123);
}

TEST_F(TimeTest, add_seconds) // NOLINT
{
  Time epoch("2000-01-01-00:00:00.123");
  epoch.add_seconds(1);
  EXPECT_EQ(epoch.get_gmtime(), "2000-01-01-00:00:01");
}

TEST_F(TimeTest, sub_seconds) // NOLINT
{
  Time epoch("2000-01-01-00:00:00.123");
  epoch.sub_seconds(1);
  EXPECT_EQ(epoch.get_gmtime(), "1999-12-31-23:59:59");
}

TEST_F(TimeTest, get_localtime) // NOLINT
{
  // determine the offset between UTC and localtime
  time_t t = time(nullptr);
  struct tm lt = {0};
  localtime_r(&t, &lt);
  std::cerr << "Offset to GMT is " << lt.tm_gmtoff << std::endl;

  Time epoch;
  epoch.set_time("2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_gmtime(), "2000-01-01-00:00:00");
  EXPECT_EQ(epoch.get_localtime(), "2000-01-01-00:00:00");
}

TEST_F(TimeTest, test_set_fractional_time) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  uint64_t attoseconds_per_decisecond = ska::pst::common::attoseconds_per_second / ska::pst::common::deciseconds_per_second;
  epoch.set_fractional_time(attoseconds_per_decisecond);
  EXPECT_EQ(epoch.get_fractional_time(), 0.1);
  EXPECT_EQ(epoch.get_fractional_time_attoseconds(), attoseconds_per_decisecond*1);

  // ensure that it can be set multiple times with different values
  epoch.set_fractional_time(attoseconds_per_decisecond*2);
  EXPECT_EQ(epoch.get_fractional_time(), 0.2);
  EXPECT_EQ(epoch.get_fractional_time_attoseconds(), attoseconds_per_decisecond*2);

  static constexpr double seconds_per_decisecond = 0.1;
  epoch.set_fractional_time(seconds_per_decisecond);
  EXPECT_EQ(epoch.get_fractional_time(), seconds_per_decisecond);
  EXPECT_EQ(epoch.get_fractional_time_attoseconds(), attoseconds_per_decisecond);
}

TEST_F(TimeTest, test_set_fractional_time_limits) // NOLINT
{
  Time epoch("2000-01-01-00:00:00");
  EXPECT_THROW(epoch.set_fractional_time(ska::pst::common::attoseconds_per_second), std::runtime_error); // NOLINT
  EXPECT_THROW(epoch.set_fractional_time(ska::pst::common::attoseconds_per_second + 1), std::runtime_error); // NOLINT
  double fractional_seconds = 1.0;
  EXPECT_THROW(epoch.set_fractional_time(fractional_seconds), std::runtime_error); // NOLINT
  EXPECT_THROW(epoch.set_fractional_time(fractional_seconds + 0.1), std::runtime_error); // NOLINT
}

} // namespace ska::pst::common::test