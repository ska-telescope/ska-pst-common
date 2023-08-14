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

#include <spdlog/spdlog.h>

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/PacketGeneratorBitDepthTest.h"
#include "ska/pst/common/utils/GaussianNoiseGenerator.h"
#include "ska/pst/common/utils/SineWaveGenerator.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

void PacketGeneratorBitDepthTest::SetUp()
{
  header.load_from_file(test_data_file("data_header.txt"));
  buffer.resize(default_buffer_size);
}

void PacketGeneratorBitDepthTest::TearDown()
{
}

TEST_P(PacketGeneratorBitDepthTest, test_generate_validate_packet) // NOLINT
{
  std::shared_ptr<TestPacketLayout> layout = std::make_shared<TestPacketLayout>();
  layout->compute_packet_properties(GetParam());
  ska::pst::common::GaussianNoiseGenerator gnd(layout);
  header.set("NBIT", GetParam());

  gnd.configure(header);

  buffer.resize(layout->get_packet_size());
  auto buffer_ptr = (&buffer[0]);
  gnd.fill_packet(buffer_ptr);
  gnd.reset();
  EXPECT_TRUE(gnd.test_packet(buffer_ptr));
  EXPECT_FALSE(gnd.test_packet(buffer_ptr));

  // perform a shift of all the values in the buffer
  for (unsigned i=0; i<buffer.size()-1; i++)
  {
    buffer_ptr[i] = buffer_ptr[i+1]; // NOLINT
  }

  gnd.reset();
  EXPECT_FALSE(gnd.test_packet(buffer_ptr));

  ska::pst::common::SineWaveGenerator swg(layout);
  swg.configure(header);

  swg.fill_packet(buffer_ptr);
  swg.reset();
  EXPECT_TRUE(swg.test_packet(buffer_ptr));
  EXPECT_FALSE(swg.test_packet(buffer_ptr));

  // perform a shift of all the values in the buffer
  for (unsigned i=0; i<buffer.size()-1; i++)
  {
    buffer_ptr[i] = buffer_ptr[i+1]; // NOLINT
  }

  swg.reset();
  EXPECT_FALSE(swg.test_packet(buffer_ptr));
}

TEST_P(PacketGeneratorBitDepthTest, test_generate_validate_blocks) // NOLINT
{
  std::shared_ptr<TestPacketLayout> layout = std::make_shared<TestPacketLayout>();
  layout->compute_packet_properties(GetParam());
  header.set("NBIT", GetParam());

  uint32_t npackets_per_spectrum = header.get_uint32("NCHAN") / header.get_uint32("NCHAN_PP");
  ASSERT_EQ(header.get_uint32("NCHAN") % header.get_uint32("NCHAN_PP"), 0);

  uint32_t buffer_size = layout->get_packet_data_size() * npackets_per_spectrum * 2;
  buffer.resize(buffer_size);
  auto buffer_ptr = (&buffer[0]);

  ska::pst::common::GaussianNoiseGenerator gnd(layout);
  gnd.configure(header);
  gnd.fill_data(buffer_ptr, buffer_size);
  gnd.reset();
  EXPECT_TRUE(gnd.test_data(buffer_ptr, buffer_size));
  EXPECT_FALSE(gnd.test_data(buffer_ptr, buffer_size));

  ska::pst::common::SineWaveGenerator swg(layout);
  swg.configure(header);
  swg.fill_data(buffer_ptr, buffer_size);
  swg.reset();
  EXPECT_TRUE(swg.test_data(buffer_ptr, buffer_size));
  EXPECT_FALSE(swg.test_data(buffer_ptr, buffer_size));
}

INSTANTIATE_TEST_SUITE_P(SignalGenerators, PacketGeneratorBitDepthTest, testing::Values(8, 16)); // NOLINT

} // namespace ska::pst::common::test
