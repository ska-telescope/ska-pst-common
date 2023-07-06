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
#include "ska/pst/common/utils/tests/DataUnpackerTest.h"
using namespace std::chrono;

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

void DataUnpackerTest::SetUp()
{
  SPDLOG_TRACE("ska::pst::common::test::DataUnpackerTest::SetUp loading data and weights headers");
  data_header.load_from_file(test_data_file("DataUnpacker_data_header.txt"));
  weights_header.load_from_file(test_data_file("DataUnpacker_weights_header.txt"));

  SPDLOG_TRACE("DataUnpackerTest::SetUp generating packed data");
  GeneratePackedData();
}

void DataUnpackerTest::TearDown()
{
}

void DataUnpackerTest::GeneratePackedData()
{
  data.resize(data_header.get_uint32("RESOLUTION"));
  weights.resize(weights_header.get_uint32("RESOLUTION"));


  const uint32_t nheaps = 1;
  const uint32_t packets_per_heap = data_header.get_uint32("NCHAN") / data_header.get_uint32("UDP_NCHAN");
  const uint32_t npol = data_header.get_uint32("NPOL");
  const uint32_t nsamp_pp = data_header.get_uint32("UDP_NSAMP");
  const uint32_t nchan_pp = data_header.get_uint32("UDP_NCHAN");
  const uint32_t ndim = data_header.get_uint32("NDIM");
  const uint32_t nchan = data_header.get_uint32("NCHAN");

  int16_t * data_ptr = reinterpret_cast<int16_t *>(&data[0]);

  SPDLOG_TRACE("ska::pst::common::test::DataUnpackerTest::GeneratePackedData generating data into {}", reinterpret_cast<void *>(data_ptr));
  uint32_t osamp = 0;
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
          for (uint32_t n=0; n<ndim; n++)
          {
            // ensure range is 0 to 32767
            int64_t value = (ochanpol * nsamp_pp) + m;
            *data_ptr = int16_t(value % 32767);
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

  // the weights stream consists of the scale and weights array from each UDP packet
  uint32_t packet_scales_size = weights_header.get_uint32("PACKET_SCALES_SIZE");
  uint32_t packet_weights_size = weights_header.get_uint32("PACKET_WEIGHTS_SIZE");
  uint32_t weight_nbit = weights_header.get_uint32("NBIT");

  uint64_t wdx = 0;
  uint32_t npackets = weights_header.get_uint32("RESOLUTION") / (packet_scales_size + packet_weights_size);
  uint32_t weights_per_packet = (packet_weights_size * 8) / weight_nbit;
  ASSERT_EQ((packet_weights_size * 8) % weight_nbit, 0);

  SPDLOG_TRACE("ska::pst::common::test::DataUnpackerTest::GeneratePackedData generating weights weights.size()={} npackets={} weights+scales={} weights_per_packet={}",
    weights.size(), npackets, packet_scales_size + packet_weights_size, weights_per_packet);
  for (uint32_t i=0; i<npackets; i++)
  {
    float * scl = reinterpret_cast<float *>(&weights[wdx]);
    *scl = get_weight_for_channel(i * nchan_pp, nchan_pp);
    wdx += packet_scales_size;

    uint16_t * wts = reinterpret_cast<uint16_t *>(&weights[wdx]);
    for (uint32_t j=0; j<weights_per_packet; j++)
    {
      wts[j] = 65535;
    }
    wdx += packet_weights_size;
  }
}

auto DataUnpackerTest::get_weight_for_channel(uint32_t channel, uint32_t nchan_per_packet) -> float
{
  if (channel / nchan_per_packet == 0)
  {
    return std::nanf("dropped");
  }
  return 1.0f;
}

TEST_F(DataUnpackerTest, test_configure) // NOLINT
{
  ska::pst::common::DataUnpacker unpacker;
  EXPECT_NO_THROW(unpacker.configure(data_header, weights_header)); // NOLINT

  static constexpr uint32_t bad_header_param = 3;
  data_header.set("NCHAN", bad_header_param);
  EXPECT_THROW(unpacker.configure(data_header, weights_header), std::runtime_error); // NOLINT
  data_header.set("NBIT", bad_header_param);
  EXPECT_THROW(unpacker.configure(data_header, weights_header), std::runtime_error); // NOLINT
  data_header.set("NPOL", bad_header_param);
  EXPECT_THROW(unpacker.configure(data_header, weights_header), std::runtime_error); // NOLINT
  data_header.set("NDIM", bad_header_param);
  EXPECT_THROW(unpacker.configure(data_header, weights_header), std::runtime_error); // NOLINT
}

TEST_F(DataUnpackerTest, test_unpack) // NOLINT
{
  ska::pst::common::DataUnpacker unpacker;
  unpacker.configure(data_header, weights_header);

  // generate packed data and weights
  std::vector<std::vector<std::vector<std::complex<float>>>>& unpacked = unpacker.unpack(&data[0], data.size(), &weights[0], weights.size());

  const uint32_t nsamp = unpacked.size();
  const uint32_t nchan = unpacked[0].size();
  const uint32_t npol = unpacked[0][0].size();
  const uint32_t nchan_per_packet = weights_header.get_uint32("UDP_NCHAN");
  const uint32_t nsamp_per_packet = weights_header.get_uint32("UDP_NSAMP");

  for (unsigned isamp=0; isamp<nsamp; isamp++)
  {
    for (unsigned ichan=0; ichan<nchan; ichan++)
    {
      for (unsigned ipol=0; ipol<npol; ipol++)
      {
        uint32_t ochanpol = ipol * nchan + ichan;
        float value = float((ochanpol * nsamp_per_packet) + isamp);
        if (std::isnan(get_weight_for_channel(ichan, nchan_per_packet)))
        {
          value = 0;
        }
        ASSERT_EQ(unpacked[isamp][ichan][ipol].real(), value);
        ASSERT_EQ(unpacked[isamp][ichan][ipol].imag(), value * -1);
      }
    }
  }
}

TEST_F(DataUnpackerTest, test_integrate_bandpass) // NOLINT
{
  ska::pst::common::DataUnpacker unpacker;
  unpacker.configure(data_header, weights_header);
  unpacker.integrate_bandpass(&data[0], data.size(), &weights[0], weights.size());
  std::vector<std::vector<float>>& bandpass = unpacker.get_bandpass();

  const uint32_t nsamp = data_header.get_uint32("UDP_NSAMP");
  const uint32_t nchan = bandpass.size();
  const uint32_t npol = bandpass[0].size();
  const uint32_t nchan_per_packet = weights_header.get_uint32("UDP_NCHAN");

  for (unsigned ichan=0; ichan<nchan; ichan++)
  {
    for (unsigned ipol=0; ipol<npol; ipol++)
    {
      const uint32_t ochanpol = ipol * nchan + ichan;
      float power = 0;
      for (unsigned isamp=0; isamp<nsamp; isamp++)
      {
        float value = float((ochanpol * nsamp) + isamp);
        power += (value * value) + (value * value);
      }
      if (std::isnan(get_weight_for_channel(ichan, nchan_per_packet)))
      {
        power = 0;
      }
      float allowed_error = power / 100000;
      EXPECT_NEAR(power, bandpass[ichan][ipol], allowed_error);
    }
  }
}

TEST_P(DataUnpackerTest, test_unpack_performance) // NOLINT
{
  std::string param = GetParam();
  std::string data_file = param + "_data_header.txt";
  std::string weights_file = param + "_weights_header.txt";
  SPDLOG_INFO("data_file: {}", data_file);
  SPDLOG_INFO("weights_file: {}", weights_file);
  data_header.load_from_file(test_data_file(data_file));
  weights_header.load_from_file(test_data_file(weights_file));

  GeneratePackedData();

  ska::pst::common::DataUnpacker unpacker;
  unpacker.configure(data_header, weights_header);

  // force a resize of the unpacked attribute in the DataUnpacker base class
  // unpacker.resize(data.size());
  std::vector<std::vector<std::vector<std::complex<float>>>>& unpacked = unpacker.unpack(&data[0], data.size(), &weights[0], weights.size());

  // Recording the timestamp at the start of the code
  auto beg = high_resolution_clock::now();
  
  // generate packed data and weights
  unpacked = unpacker.unpack(&data[0], data.size(), &weights[0], weights.size());
  // const uint32_t nsamp = unpacked.size();
  // const uint32_t nchan = unpacked[0].size();
  // const uint32_t npol = unpacked[0][0].size();
  // const uint32_t nchan_per_packet = weights_header.get_uint32("UDP_NCHAN");
  // const uint32_t nsamp_per_packet = weights_header.get_uint32("UDP_NSAMP");

  // for (unsigned isamp=0; isamp<nsamp; isamp++)
  // {
  //   for (unsigned ichan=0; ichan<nchan; ichan++)
  //   {
  //     for (unsigned ipol=0; ipol<npol; ipol++)
  //     {
  //       uint32_t ochanpol = ipol * nchan + ichan;
  //       float value = float((ochanpol * nsamp_per_packet) + isamp);
  //       if (std::isnan(get_weight_for_channel(ichan, nchan_per_packet)))
  //       {
  //         value = 0;
  //       }
  //     }
  //   }
  // }

  // Taking a timestamp after the code is ran
  auto end = high_resolution_clock::now();
  // Subtracting the end timestamp from the beginning
  // And we choose to receive the difference in microseconds
  auto duration = duration_cast<microseconds>(end - beg);

  // Displaying the elapsed time
  SPDLOG_INFO("Elapsed Time (microseconds): {}", duration.count());

}

/*
TODO
- parse directory for all configuration files present in test/utils/data
- instansiate tests for each configuration file detected
*/
// nchan * nbit * ndim * npol * nsamp / 8
INSTANTIATE_TEST_SUITE_P(PerformanceTests, DataUnpackerTest, testing::Values(
"Low_AA0.5"
)); // NOLINT

} // namespace ska::pst::common::test
