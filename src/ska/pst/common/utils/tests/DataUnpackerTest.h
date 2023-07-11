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
#include <chrono>
#include <filesystem>

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/utils/DataUnpacker.h"

#ifndef SKA_PST_COMMON_UTILS_TESTS_DataUnpackerTest_h
#define SKA_PST_COMMON_UTILS_TESTS_DataUnpackerTest_h

namespace ska::pst::common::test {

  /**
   * @brief Test the DataUnpacker class
   *
   * @details
   *
   */
  class DataUnpackerTest : public ::testing::TestWithParam<const char*>
  {
    protected:
      void SetUp() override;

      void TearDown() override;

      void GeneratePackedData(std::string data_header_file, std::string weights_heade_file);

      float get_weight_for_channel(uint32_t channel, uint32_t nchan_per_packet);

      float get_float_value_for_channel_sample(uint32_t ichan, uint32_t nsamp, uint32_t isamp, uint32_t nbit);

      template <typename T>
      void GenerateQuantisedPackedData(T * data_ptr)
      {
        const uint32_t nheaps = data_header.get_uint32("DB_BUFSZ")/data_header.get_uint32("RESOLUTION");
        const uint32_t packets_per_heap = data_header.get_uint32("NCHAN") / data_header.get_uint32("UDP_NCHAN");
        const uint32_t npol = data_header.get_uint32("NPOL");
        const uint32_t nsamp_pp = data_header.get_uint32("UDP_NSAMP");
        const uint32_t nchan_pp = data_header.get_uint32("UDP_NCHAN");
        const uint32_t ndim = data_header.get_uint32("NDIM");
        const uint32_t nchan = data_header.get_uint32("NCHAN");
        
        SPDLOG_TRACE("ska::pst::common::test::DataUnpackerTest::GenerateQuantisedPackedData generating data into {}", reinterpret_cast<void *>(data_ptr));
        uint32_t osamp = 0;
        uint32_t nsamp = nheaps * nsamp_pp;
        for (uint32_t i=0; i<nheaps; i++)
        {
          for (uint32_t j=0; j<packets_per_heap; j++)
          {
            for (uint32_t k=0; k<npol; k++)
            {
              for (uint32_t l=0; l<nchan_pp; l++)
              {
                uint32_t ochan = j * nchan_pp + l;
                uint32_t ochanpol = (k * nchan) + ochan;
                for (uint32_t m=0; m<nsamp_pp; m++)
                {
                  uint32_t osamp = i * nsamp_pp + m;
                  for (uint32_t n=0; n<ndim; n++)
                  {
                    get_value_for_channel_sample(ochan, nsamp, osamp, data_ptr);
                    if (n == 1)
                    {
                      *data_ptr *= -1;
                    }
                    data_ptr++;
                  }
                }
              }
            }
          }
        }
      }

      template <typename T>
      void get_value_for_channel_sample(uint32_t ichan, uint32_t nsamp, uint32_t isamp, T * data_ptr)
      {
        *data_ptr = T((ichan * nsamp + isamp) % 255);
      }

    public:
      DataUnpackerTest() = default;

      ~DataUnpackerTest() = default;

      ska::pst::common::AsciiHeader data_header;

      ska::pst::common::AsciiHeader weights_header;

      std::vector<char> data;

      std::vector<char> weights;

    private:

  };

} // namespace ska::pst::common::test

#endif // SKA_PST_COMMON_UTILS_TESTS_DataUnpackerTest_h
