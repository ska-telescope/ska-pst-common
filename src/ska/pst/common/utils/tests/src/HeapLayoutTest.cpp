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

#include "ska/pst/common/utils/tests/HeapLayoutTest.h"
#include "ska/pst/common/testutils/GtestMain.h"

#include <spdlog/spdlog.h>

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

unsigned constexpr nchan = 1024;
unsigned constexpr udp_nsamp = 128;
unsigned constexpr udp_nchan = 64;   // nchan must be a multiple of udp_nchan
unsigned constexpr wt_nsamp = 32;    // udp_nsamp must be a multiple of wt_nsamp

unsigned constexpr data_nbit = 8;
unsigned constexpr data_npol = 2;
unsigned constexpr data_ndim = 2;

unsigned constexpr weights_nbit = 16;
unsigned constexpr weights_npol = 1;
unsigned constexpr weights_ndim = 1;

void HeapLayoutTest::SetUp()
{
  // Define a minimal good data header
  data_header.set("NCHAN",nchan);
  data_header.set("NBIT",data_nbit);
  data_header.set("NPOL",data_npol);
  data_header.set("NDIM",data_ndim);

  data_header.set("UDP_NSAMP",udp_nsamp);
  data_header.set("UDP_NCHAN",udp_nchan);
  data_header.set("WT_NSAMP",wt_nsamp);

  // Define a minimal good weights header
  weights_header = data_header;
  weights_header.set("NBIT",weights_nbit);
  weights_header.set("NPOL",weights_npol);
  weights_header.set("NDIM",weights_ndim);
}

void HeapLayoutTest::TearDown()
{
}

TEST_F(HeapLayoutTest, test_configure) // NOLINT
{
  EXPECT_NO_THROW(layout.configure(data_header, weights_header)); // NOLINT

  auto expected_packets_per_heap = nchan / udp_nchan;
  EXPECT_EQ(layout.get_packets_per_heap(), expected_packets_per_heap);

  auto nbyte_per_datum = (data_nbit*data_npol*data_ndim)/ska::pst::common::bits_per_byte;
  EXPECT_EQ(layout.get_data_packet_stride(), nbyte_per_datum * udp_nsamp * udp_nchan);
  EXPECT_EQ(layout.get_data_heap_stride(), nbyte_per_datum * udp_nsamp * nchan);

  auto nbyte_per_weight = (weights_nbit*weights_npol*weights_ndim)/ska::pst::common::bits_per_byte;
  auto nweight_per_channel = udp_nsamp/wt_nsamp;
  auto nbyte_per_scale = sizeof(float);
  EXPECT_EQ(layout.get_weights_packet_stride(), nbyte_per_weight * nweight_per_channel * udp_nchan + nbyte_per_scale);
  EXPECT_EQ(layout.get_weights_heap_stride(), nbyte_per_weight * nweight_per_channel * nchan + nbyte_per_scale * expected_packets_per_heap);
}

TEST_F(HeapLayoutTest, test_inconsistent_nchan) // NOLINT
{
  ska::pst::common::AsciiHeader inconsistent_weights_header(weights_header);
  inconsistent_weights_header.set("NCHAN", nchan * 2); // NOLINT
  EXPECT_THROW(layout.configure(data_header, inconsistent_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_inconsistent_udp_nsamp) // NOLINT
{
  ska::pst::common::AsciiHeader inconsistent_weights_header(weights_header);
  inconsistent_weights_header.set("UDP_NSAMP", udp_nsamp * 2); // NOLINT
  EXPECT_THROW(layout.configure(data_header, inconsistent_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_inconsistent_udp_nchan) // NOLINT
{
  ska::pst::common::AsciiHeader inconsistent_weights_header(weights_header);
  inconsistent_weights_header.set("UDP_NCHAN", udp_nchan * 2); // NOLINT
  EXPECT_THROW(layout.configure(data_header, inconsistent_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_inconsistent_wt_nsamp) // NOLINT
{
  ska::pst::common::AsciiHeader inconsistent_weights_header(weights_header);
  inconsistent_weights_header.set("WT_NSAMP", wt_nsamp * 2); // NOLINT
  EXPECT_THROW(layout.configure(data_header, inconsistent_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_nchan) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_data_header(data_header);
  ska::pst::common::AsciiHeader invalid_weights_header(weights_header);

  // nchan not divisible by udp_nchan
  invalid_data_header.set("NCHAN", udp_nchan / 2); // NOLINT
  invalid_weights_header.set("NCHAN", udp_nchan / 2); // NOLINT
  EXPECT_THROW(layout.configure(invalid_data_header, invalid_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_udp_nsamp) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_data_header(data_header);
  ska::pst::common::AsciiHeader invalid_weights_header(weights_header);
  
  // udp_nsamp not divisible by wt_nsamp
  invalid_data_header.set("UDP_NSAMP", wt_nsamp / 2); // NOLINT
  invalid_weights_header.set("UDP_NSAMP", wt_nsamp / 2); // NOLINT
  EXPECT_THROW(layout.configure(invalid_data_header, invalid_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_data_nbit) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_data_header(data_header);
  // nbit not an expected value
  invalid_data_header.set("NBIT", 24); // NOLINT
  EXPECT_THROW(layout.configure(invalid_data_header, weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_data_ndim) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_data_header(data_header);
  // ndim not equal to 2
  invalid_data_header.set("NDIM", 4); // NOLINT
  EXPECT_THROW(layout.configure(invalid_data_header, weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_data_npol) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_data_header(data_header);
  // npol not equal to 2
  invalid_data_header.set("NPOL", 4); // NOLINT
  EXPECT_THROW(layout.configure(invalid_data_header, weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_weights_ndim) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_weights_header(weights_header);
  // ndim not equal to 1
  invalid_weights_header.set("NDIM", 2); // NOLINT
  EXPECT_THROW(layout.configure(data_header, invalid_weights_header), std::runtime_error); // NOLINT
}

TEST_F(HeapLayoutTest, test_invalid_weights_npol) // NOLINT
{
  ska::pst::common::AsciiHeader invalid_weights_header(weights_header);
  // npol not equal to 1
  invalid_weights_header.set("NPOL", 2); // NOLINT
  EXPECT_THROW(layout.configure(data_header, invalid_weights_header), std::runtime_error); // NOLINT
}

} // namespace ska::pst::common::test
