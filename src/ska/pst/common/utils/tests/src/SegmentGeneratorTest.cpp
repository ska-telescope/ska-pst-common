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

TEST_F(SegmentGeneratorTest, test_bad_configure) // NOLINT
{
  auto generator = std::make_shared<ska::pst::common::SegmentGenerator>();

  // no DATA_GENERATOR specified
  EXPECT_THROW(generator->configure(data_header, weights_header), std::runtime_error); // NOLINT
}

TEST_P(SegmentGeneratorTest, test_configure) // NOLINT
{
  auto generator = std::make_shared<ska::pst::common::SegmentGenerator>();
  data_header.set_val("DATA_GENERATOR", GetParam());

  EXPECT_NO_THROW(generator->configure(data_header, weights_header)); // NOLINT

  AsciiHeader generated_data_header = generator->get_data_header();
  AsciiHeader generated_weights_header = generator->get_weights_header();

  // delete additional parameters initialised by the SegmentGenerator if not in the input data_header
  for (auto param: {"RESOLUTION", "FILE_NUMBER"})
  { 
    if (!data_header.has(param))
    {
      SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_configure deleting {} from generated data_header",param);
      generated_data_header.del(param);
    }
  }

  // delete additional parameters initialised by the SegmentGenerator, which are not in the input weights_header
  for (auto param: {"RESOLUTION", "PACKET_WEIGHTS_SIZE", "PACKET_SCALES_SIZE", "BLOCK_HEADER_BYTES", "BLOCK_DATA_BYTES"})
  {
    if (!weights_header.has(param))
    {
      SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_configure deleting {} from generated weights_header",param);
      generated_weights_header.del(param);
    }
  }

  EXPECT_EQ(data_header,generated_data_header);
  EXPECT_EQ(weights_header,generated_weights_header);
}

TEST_P(SegmentGeneratorTest, test_generate_validate) // NOLINT
{
  auto generator = std::make_shared<ska::pst::common::SegmentGenerator>();
  data_header.set_val("DATA_GENERATOR", GetParam());
  generator->configure(data_header, weights_header);
  generator->resize(default_nheap);

  SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_generate_validate generator->next_segment()");
  auto segment = generator->next_segment();
  generator->reset();

  SPDLOG_TRACE("ska::pst::common::test::SegmentGeneratorTest::test_generate_validate generator->test_segment()");
  EXPECT_TRUE(generator->test_segment(segment));
}

INSTANTIATE_TEST_SUITE_P(SignalGenerators, SegmentGeneratorTest, testing::Values("Random", "Sine", "GaussianNoise")); // NOLINT

} // namespace ska::pst::common::test
