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

#include "ska/pst/common/utils/tests/SegmentGeneratorTest.h"
#include "ska/pst/common/testutils/GtestMain.h"

#include <spdlog/spdlog.h>

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

void SegmentGeneratorTest::SetUp()
{
  data_header.load_from_file(test_data_file("SegmentGenerator_data_header.txt"));
  weights_header.load_from_file(test_data_file("SegmentGenerator_weights_header.txt"));
}

void SegmentGeneratorTest::TearDown()
{
}

TEST_P(SegmentGeneratorTest, test_configure) // NOLINT
{
  auto generator = std::make_shared<ska::pst::common::SegmentGenerator>();
  data_header.set_val("DATA_GENERATOR", GetParam());

  EXPECT_NO_THROW(generator->configure(data_header, weights_header)); // NOLINT

  static constexpr uint32_t bad_header_param = 3;

  ska::pst::common::AsciiHeader bad_data_header(data_header);
  bad_data_header.set("NCHAN", bad_header_param);
  EXPECT_THROW(generator->configure(bad_data_header, weights_header), std::runtime_error); // NOLINT

  ska::pst::common::AsciiHeader bad_weights_header(weights_header);
  bad_weights_header.set("NPOL", bad_header_param);
  EXPECT_THROW(generator->configure(data_header, bad_weights_header), std::runtime_error); // NOLINT
}


TEST_P(SegmentGeneratorTest, test_generate_validate) // NOLINT
{
  auto generator = std::make_shared<ska::pst::common::SegmentGenerator>();
  data_header.set_val("DATA_GENERATOR", GetParam());
  generator->configure(data_header, weights_header);
  generator->resize(default_nheap);

  SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_generate_validate dg->fill_data()");
  auto segment = generator->next_segment();

  auto packet_generator = generator->get_packet_generator();
  packet_generator->reset();

  SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_generate_validate packet_generator->test_data()");
  EXPECT_TRUE(packet_generator->test_data(segment.data.block, segment.data.size));
  SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_generate_validate packet_generator->test_weights()");
  EXPECT_TRUE(packet_generator->test_weights(segment.weights.block, segment.weights.size));
  SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_generate_validate packet_generator->test_scales()");
  EXPECT_TRUE(packet_generator->test_scales(segment.weights.block, segment.weights.size));
}

INSTANTIATE_TEST_SUITE_P(SignalGenerators, SegmentGeneratorTest, testing::Values("Random", "Sine", "GaussianNoise")); // NOLINT

} // namespace ska::pst::common::test
