/*
 * Copyright 2023 Square Kilometre Array Observatory
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

#include <numeric>
#include <spdlog/spdlog.h>

#include "ska/pst/common/definitions.h"
#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/NormalSequenceTest.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

NormalSequenceTest::NormalSequenceTest()
    : ::testing::Test()
{
}

void NormalSequenceTest::SetUp()
{
  buffer.resize(default_buffer_size);
}

void NormalSequenceTest::TearDown()
{
}

void NormalSequenceTest::compute_mean_stddev(const std::vector<float>& values, float * mean, float *stddev)
{
  double sum = std::accumulate(values.begin(), values.end(), 0.0);
  *mean = sum / values.size(); // NOLINT
  double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
  *stddev = std::sqrt(sq_sum / values.size() - *mean * *mean); // NOLINT
}

void NormalSequenceTest::assert_mean_stddev(const std::vector<float>& values, float expected_mean, float expected_stddev)
{
  float mean{0}, stddev{0};
  compute_mean_stddev(values, &mean, &stddev);
  ASSERT_NEAR(expected_mean, mean, 1e-1);
  ASSERT_NEAR(expected_stddev, stddev, 1e-1);
}

TEST_F(NormalSequenceTest, test_generate_validate) // NOLINT
{
  data_header.load_from_file(test_data_file("8bit_data_header.txt"));
  weights_header.load_from_file(test_data_file("8bit_weights_header.txt"));
  NormalSequence ns;

  ns.configure(data_header);
  ns.generate(&buffer[0], buffer.size());
  ns.reset();
  EXPECT_TRUE(ns.validate(&buffer[0], buffer.size()));
  EXPECT_FALSE(ns.validate(&buffer[0], buffer.size()));

  // perform a shift of all the values in the buffer
  for (unsigned i=0; i<buffer.size()-1; i++)
  {
    buffer[i] = buffer[i+1]; // NOLINT
  }

  ns.reset();
  EXPECT_FALSE(ns.validate(&buffer[0], buffer.size()));
}

TEST_F(NormalSequenceTest, test_generate_8bit) // NOLINT
{
  data_header.load_from_file(test_data_file("8bit_data_header.txt"));
  weights_header.load_from_file(test_data_file("8bit_weights_header.txt"));

  NormalSequence ns;

  float expected_mean = data_header.get_float("NORMAL_DIST_MEAN");
  float expected_stddev = data_header.get_float("NORMAL_DIST_STDDEV");
  uint32_t nbit = data_header.get_uint32("NBIT");

  ns.configure(data_header);
  SPDLOG_TRACE("ska::pst::common::test::NormalSequenceTest::test_generate_8bit generating {} bytes of data", buffer.size());
  ns.generate(&buffer[0], buffer.size());

  // measure the mean and standard deviation of the generated timeseries
  std::vector<float> unpacked(buffer.size());
  auto buffer_int8 = reinterpret_cast<int8_t*>(&buffer[0]);
  const unsigned buffer_nval = (buffer.size() * ska::pst::common::bits_per_byte) / nbit;
  for (unsigned i=0; i<buffer_nval; i++)
  {
    unpacked[i] = static_cast<float>(buffer_int8[i]); // NOLINT
  }
  assert_mean_stddev(unpacked, expected_mean, expected_stddev);
}

TEST_F(NormalSequenceTest, test_generate_16bit) // NOLINT
{
  data_header.load_from_file(test_data_file("16bit_data_header.txt"));
  weights_header.load_from_file(test_data_file("16bit_weights_header.txt"));

  NormalSequence ns;

  float expected_mean = data_header.get_float("NORMAL_DIST_MEAN");
  float expected_stddev = data_header.get_float("NORMAL_DIST_STDDEV");
  uint32_t nbit = data_header.get_uint32("NBIT");

  ns.configure(data_header);
  SPDLOG_TRACE("ska::pst::common::test::NormalSequenceTest::test_generate_16bit generating {} bytes of data", buffer.size());
  ns.generate(&buffer[0], buffer.size());

  auto buffer_int16 = reinterpret_cast<int16_t*>(&buffer[0]);
  const unsigned buffer_nval = (buffer.size() * ska::pst::common::bits_per_byte) / nbit;
  std::vector<float> unpacked(buffer_nval);
  for (unsigned i=0; i<buffer_nval; i++)
  {
    unpacked[i] = static_cast<float>(buffer_int16[i]); // NOLINT
  }

  // measure the mean and standard deviation of the generated timeseries
  assert_mean_stddev(unpacked, expected_mean, expected_stddev);
}

TEST_F(NormalSequenceTest, test_generate_red_noise) // NOLINT
{
  data_header.load_from_file(test_data_file("16bit_data_header.txt"));
  weights_header.load_from_file(test_data_file("16bit_weights_header.txt"));

  NormalSequence ns;

  // set the header parameter that configures the red noise stddev
  static constexpr float expected_mean = 0.0;
  static constexpr float expected_stddev = 10.0;
  static constexpr float red_stddev = 1.0;
  data_header.set("NORMAL_DIST_MEAN", expected_mean);
  data_header.set("NORMAL_DIST_STDDEV", expected_stddev);
  data_header.set("NORMAL_DIST_RED_STDDEV", red_stddev);

  uint32_t nbit = data_header.get_uint32("NBIT");

  ns.configure(data_header);
  SPDLOG_TRACE("ska::pst::common::test::NormalSequenceTest::test_generate_red_noise generating {} bytes of data", buffer.size());
  ns.generate(&buffer[0], buffer.size());

  auto buffer_int16 = reinterpret_cast<int16_t*>(&buffer[0]);
  const unsigned buffer_nval = (buffer.size() * ska::pst::common::bits_per_byte) / nbit;
  std::vector<float> unpacked(buffer_nval);
  for (unsigned i=0; i<buffer_nval; i++)
  {
    unpacked[i] = static_cast<float>(buffer_int16[i]); // NOLINT
  }

  float mean{0}, stddev{0};
  compute_mean_stddev(unpacked, &mean, &stddev);
  SPDLOG_TRACE("ska::pst::common::test::NormalSequenceTest::test_generate_red_noise mean={} stddev={}", mean, stddev);

  // mean should remain at 0 in the presence of red noise
  ASSERT_NEAR(expected_mean, mean, 1e-1);
  ASSERT_NEAR(expected_stddev, stddev, 99);
}

} // namespace ska::pst::common::test

