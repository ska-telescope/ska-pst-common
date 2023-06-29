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

#include <gtest/gtest.h>
#include <vector>

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/utils/DataGenerator.h"

#ifndef SKA_PST_COMMON_UTILS_TESTS_DataGeneratorBitDepthTest_h
#define SKA_PST_COMMON_UTILS_TESTS_DataGeneratorBitDepthTest_h

namespace ska::pst::common::test {

  /**
   * @brief Test the DataGenerator class
   *
   * @details
   *
   */
  class DataGeneratorBitDepthTest : public ::testing::TestWithParam<int>
  {
    protected:
      void SetUp() override;

      void TearDown() override;

    public:
      DataGeneratorBitDepthTest() = default;

      ~DataGeneratorBitDepthTest() = default;

      ska::pst::common::AsciiHeader header;

      std::vector<char> buffer;

    private:

      uint32_t default_buffer_size{1024};

  };

  class TestDataLayout : public ska::pst::common::DataLayout
  {
    public:
    TestDataLayout ()
    {
      header.load_from_file(test_data_file("data_header.txt"));
      nsamp_per_packet = header.get_uint32("NSAMP_PP");
      nchan_per_packet = header.get_uint32("NCHAN_PP");
      compute_packet_properties(header.get_uint32("NBIT"));
    }

    void compute_packet_properties(uint32_t nbit)
    {
      uint32_t ndim = header.get_uint32("NDIM");
      uint32_t npol = header.get_uint32("NPOL");

      unsigned offset = 0;
      packet_header_size = 128; // NOLINT
      offset += packet_header_size;

      packet_weights_size = 512; // NOLINT
      packet_weights_offset = offset;
      offset += packet_weights_size;

      packet_scales_size = 32; // NOLINT
      packet_scales_offset = offset;
      offset += packet_scales_size;

      static constexpr uint32_t nbits_per_byte = 8;
      packet_data_size = nsamp_per_packet * nchan_per_packet * ndim * npol * nbit / nbits_per_byte;
      packet_data_offset = offset;
      offset += packet_data_size;

      packet_size = offset + packet_scales_size;
    }

    ska::pst::common::AsciiHeader header;
  };

} // namespace ska::pst::common::test

#endif // SKA_PST_COMMON_UTILS_TESTS_DataGeneratorBitDepthTest_h
