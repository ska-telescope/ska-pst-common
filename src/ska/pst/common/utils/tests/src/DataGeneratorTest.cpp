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

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/DataGeneratorTest.h"
#include "ska/pst/common/utils/DataGeneratorFactory.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

void DataGeneratorTest::SetUp()
{
  header.load_from_file(test_data_file("data_header.txt"));
  buffer.resize(default_buffer_size);
}

void DataGeneratorTest::TearDown()
{
}

TEST_F(DataGeneratorTest, test_factory) // NOLINT
{
  EXPECT_EQ(ska::pst::common::get_supported_data_generators_list(), "Random, Sine, GaussianNoise");
  std::vector<std::string> data_generators = ska::pst::common::get_supported_data_generators();
  EXPECT_EQ(data_generators[0], "Random");
  EXPECT_EQ(data_generators[1], "Sine");
  EXPECT_EQ(data_generators[2], "GaussianNoise");

  std::shared_ptr<TestDataLayout> layout = std::make_shared<TestDataLayout>();
  EXPECT_THROW(DataGeneratorFactory("Garbage", layout), std::runtime_error); // NOLINT);
}

TEST_P(DataGeneratorTest, test_configure) // NOLINT
{
  std::shared_ptr<TestDataLayout> layout = std::make_shared<TestDataLayout>();
  std::shared_ptr<ska::pst::common::DataGenerator> dg = DataGeneratorFactory(GetParam(), layout);
  EXPECT_NO_THROW(dg->configure(header)); // NOLINT

  static constexpr uint32_t bad_header_param = 3;
  header.set("NCHAN", bad_header_param);
  EXPECT_THROW(dg->configure(header), std::runtime_error); // NOLINT
  header.set("NPOL", bad_header_param);
  EXPECT_THROW(dg->configure(header), std::runtime_error); // NOLINT
  header.set("NDIM", bad_header_param);
  EXPECT_THROW(dg->configure(header), std::runtime_error); // NOLINT
}

TEST_P(DataGeneratorTest, test_generate_validate_packet) // NOLINT
{
  std::shared_ptr<TestDataLayout> layout = std::make_shared<TestDataLayout>();
  std::shared_ptr<ska::pst::common::DataGenerator> dg = DataGeneratorFactory(GetParam(), layout);

  dg->configure(header);

  buffer.resize(layout->get_packet_size());
  auto buffer_ptr = (&buffer[0]);
  dg->fill_packet(buffer_ptr);
  dg->reset();
  EXPECT_TRUE(dg->test_packet(buffer_ptr));
  EXPECT_FALSE(dg->test_packet(buffer_ptr));

  // perform a shift of all the values in the buffer
  for (unsigned i=0; i<buffer.size()-1; i++)
  {
    buffer_ptr[i] = buffer_ptr[i+1]; // NOLINT
  }

  dg->reset();
  EXPECT_FALSE(dg->test_packet(buffer_ptr));
}

TEST_P(DataGeneratorTest, test_generate_validate_blocks) // NOLINT
{
  std::shared_ptr<TestDataLayout> layout = std::make_shared<TestDataLayout>();
  std::shared_ptr<ska::pst::common::DataGenerator> dg = DataGeneratorFactory(GetParam(), layout);

  dg->configure(header);

  uint32_t npackets_per_spectrum = header.get_uint32("NCHAN") / header.get_uint32("NCHAN_PP");
  uint32_t buffer_size = layout->get_packet_data_size() * npackets_per_spectrum * 2;
  uint32_t weights_scales_size = (layout->get_packet_weights_size() + layout->get_packet_scales_size()) * npackets_per_spectrum * 2;

  buffer.resize(buffer_size);
  auto buffer_ptr = (&buffer[0]);

  SPDLOG_TRACE("ska::pst::common::test::DataGeneratorTest::test_generate_validate_blocks dg->fill_data()");
  dg->fill_data(buffer_ptr, buffer_size);
  dg->reset();
  EXPECT_TRUE(dg->test_data(buffer_ptr, buffer_size));

  // reuse the buffer for the weights and scales
  buffer.resize(weights_scales_size);
  dg->reset();
  SPDLOG_TRACE("ska::pst::common::test::DataGeneratorTest::test_generate_validate_blocks dg->fill_weights()");
  dg->fill_weights(buffer_ptr, weights_scales_size);
  SPDLOG_TRACE("ska::pst::common::test::DataGeneratorTest::test_generate_validate_blocks dg->fill_scales()");
  dg->fill_scales(buffer_ptr, weights_scales_size);
  dg->reset();
  SPDLOG_TRACE("ska::pst::common::test::DataGeneratorTest::test_generate_validate_blocks dg->test_weights()");
  EXPECT_TRUE(dg->test_weights(buffer_ptr, weights_scales_size));
  SPDLOG_TRACE("ska::pst::common::test::DataGeneratorTest::test_generate_validate_blocks dg->test_scales()");
  EXPECT_TRUE(dg->test_scales(buffer_ptr, weights_scales_size));
}

INSTANTIATE_TEST_SUITE_P(SignalGenerators, DataGeneratorTest, testing::Values("Random", "Sine", "GaussianNoise")); // NOLINT


} // namespace ska::pst::common::test
