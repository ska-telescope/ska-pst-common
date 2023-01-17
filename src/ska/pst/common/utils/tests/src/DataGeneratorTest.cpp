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
  spdlog::set_level(spdlog::level::debug);
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

TEST_P(DataGeneratorTest, test_layout_not_configured) // NOLINT
{
  std::shared_ptr<ska::pst::common::DataGenerator> dg = DataGeneratorFactory(GetParam());

  auto buffer_ptr = (&buffer[0]);

  dg->configure(header);
  EXPECT_THROW(dg->fill_block(buffer_ptr), std::runtime_error); // NOLINT
}

class TestDataLayout : public ska::pst::common::DataLayout
{
  public:
  TestDataLayout ()
  {
    unsigned offset = 0;
    packet_header_size = 100; // NOLINT
    offset += packet_header_size;

    packet_data_size = 5000; // NOLINT
    packet_data_offset = offset;
    offset += packet_data_size;

    packet_weights_size = 500; // NOLINT
    packet_weights_offset = offset;
    offset += packet_weights_size;

    packet_scales_size = 50; // NOLINT
    packet_scales_offset = offset;

    packet_size = offset + packet_scales_size;
  }
};

TEST_P(DataGeneratorTest, test_generate_validate) // NOLINT
{
  std::shared_ptr<ska::pst::common::DataGenerator> dg = DataGeneratorFactory(GetParam());
  dg->configure(header);

  TestDataLayout layout;
  dg->copy_layout(&layout);

  buffer.resize(layout.get_packet_size());
  auto buffer_ptr = (&buffer[0]);
  dg->fill_block(buffer_ptr);
  dg->reset();
  EXPECT_TRUE(dg->test_block(buffer_ptr));
  EXPECT_FALSE(dg->test_block(buffer_ptr));

  // perform a shift of all the values in the buffer
  for (unsigned i=0; i<buffer.size()-1; i++)
  {
    buffer_ptr[i] = buffer_ptr[i+1]; // NOLINT
  }

  dg->reset();
  EXPECT_FALSE(dg->test_block(buffer_ptr));
}

INSTANTIATE_TEST_SUITE_P(SignalGenerators,
                         DataGeneratorTest,
                         testing::Values("Random", "Sine"));

} // namespace ska::pst::common::test
