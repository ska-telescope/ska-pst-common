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

void HeapLayoutTest::SetUp()
{
  // Define a minimal good data header
  data_header.set("NCHAN",nchan);
  data_header.set("NBIT",8); // NOLINT
  data_header.set("NPOL",2); // NOLINT
  data_header.set("NDIM",2); // NOLINT

  data_header.set("UDP_NSAMP",udp_nsamp);
  data_header.set("UDP_NCHAN",udp_nchan);
  data_header.set("WT_NSAMP",wt_nsamp);

  // Define a minimal good weights header
  weights_header = data_header;
  weights_header.set("NBIT",16); // NOLINT
  weights_header.set("NPOL",1); // NOLINT
  weights_header.set("NDIM",1); // NOLINT
}

void HeapLayoutTest::TearDown()
{
}

TEST_F(HeapLayoutTest, test_configure) // NOLINT
{
  EXPECT_NO_THROW(layout.configure(data_header, weights_header)); // NOLINT
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
