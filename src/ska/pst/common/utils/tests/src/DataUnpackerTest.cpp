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
#include "ska/pst/common/definitions.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

void DataUnpackerTest::SetUp()
{
  SPDLOG_TRACE("DataUnpackerTest::SetUp generating packed data");
}

void DataUnpackerTest::TearDown()
{
}

void DataUnpackerTest::GeneratePackedData(const std::string& data_header_file, const std::string& weights_header_file)
{
  SPDLOG_TRACE("ska::pst::common::test::DataUnpackerTest::SetUp loading data and weights headers");
  data_header.load_from_file(test_data_file(data_header_file));
  weights_header.load_from_file(test_data_file(weights_header_file));

  data.resize(data_header.get_uint32("DB_BUFSZ"));
  weights.resize(weights_header.get_uint32("WB_BUFSZ"));

  const uint32_t nheaps = data_header.get_uint32("DB_BUFSZ")/data_header.get_uint32("RESOLUTION");
  const uint32_t nbit = data_header.get_uint32("NBIT");
  const uint32_t nchan_pp = data_header.get_uint32("UDP_NCHAN");
 
  if (nbit == 8) // NOLINT
  {
    GenerateQuantisedPackedData(reinterpret_cast<int8_t*>(&data[0]));
  }
  else if (nbit == 16) // NOLINT
  {
    GenerateQuantisedPackedData(reinterpret_cast<int16_t*>(&data[0]));
  }

  // the weights stream consists of the scale and weights array from each UDP packet
  uint32_t packet_scales_size = weights_header.get_uint32("PACKET_SCALES_SIZE");
  uint32_t packet_weights_size = weights_header.get_uint32("PACKET_WEIGHTS_SIZE");
  uint32_t weight_nbit = weights_header.get_uint32("NBIT");

  uint64_t wdx = 0;
  uint32_t npackets = weights_header.get_uint32("RESOLUTION") / (packet_scales_size + packet_weights_size);
  uint32_t weights_per_packet = (packet_weights_size * ska::pst::common::bits_per_byte) / weight_nbit;
  ASSERT_EQ((packet_weights_size * ska::pst::common::bits_per_byte) % weight_nbit, 0);

  SPDLOG_TRACE("ska::pst::common::test::DataUnpackerTest::GeneratePackedData generating weights weights.size()={} npackets={} weights+scales={} weights_per_packet={}",
    weights.size(), npackets, packet_scales_size + packet_weights_size, weights_per_packet);
  for (uint32_t i=0; i<nheaps; i++)
  {
    for (uint32_t j=0; j<npackets; j++)
    {
      auto scl = reinterpret_cast<float *>(&weights[wdx]);
      *scl = get_weight_for_channel(j * nchan_pp, nchan_pp);
      wdx += packet_scales_size;

      auto wts = reinterpret_cast<uint16_t *>(&weights[wdx]);
      for (uint32_t k=0; k<weights_per_packet; k++)
      {
        wts[k] = 65535; // NOLINT
      }
      wdx += packet_weights_size;
    }
  }
}

auto DataUnpackerTest::get_float_value_for_channel_sample(uint32_t ichan, uint32_t nsamp, uint32_t isamp, uint32_t nbit) -> float
{
  if (nbit == 8) // NOLINT
  {
    int8_t packed = 0;
    get_value_for_channel_sample(ichan, nsamp, isamp, &packed);
    return static_cast<float>(packed);
  }
  if (nbit == 16) // NOLINT
  {
    int16_t packed = 0;
    get_value_for_channel_sample(ichan, nsamp, isamp, &packed);
    return static_cast<float>(packed);
  }
  return std::nanf("badquant");
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
  GeneratePackedData("DataUnpacker_data_header.txt", "DataUnpacker_weights_header.txt");
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

  GeneratePackedData("DataUnpacker_data_header.txt", "DataUnpacker_weights_header.txt");
  unpacker.configure(data_header, weights_header);

  // generate packed data and weights
  std::vector<std::vector<std::vector<std::complex<float>>>>& unpacked = unpacker.unpack(&data[0], data.size(), &weights[0], weights.size());

  const uint32_t nsamp = unpacked.size();
  const uint32_t nchan = unpacked[0].size();
  const uint32_t npol = unpacked[0][0].size();
  const uint32_t nchan_per_packet = weights_header.get_uint32("UDP_NCHAN");
  const uint32_t nbit = data_header.get_uint32("NBIT");

  for (unsigned isamp=0; isamp<nsamp; isamp++)
  {
    for (unsigned ichan=0; ichan<nchan; ichan++)
    {
      for (unsigned ipol=0; ipol<npol; ipol++)
      {
        float value = get_float_value_for_channel_sample(ichan, nsamp, isamp, nbit);
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

  GeneratePackedData("DataUnpacker_data_header.txt", "DataUnpacker_weights_header.txt");
  unpacker.configure(data_header, weights_header);
  unpacker.integrate_bandpass(&data[0], data.size(), &weights[0], weights.size());
  std::vector<std::vector<float>>& bandpass = unpacker.get_bandpass();

  const uint32_t nheap = data.size()/data_header.get_uint32("RESOLUTION");
  const uint32_t nsamp = nheap * data_header.get_uint32("UDP_NSAMP");
  const uint32_t nbit = data_header.get_uint32("NBIT");
  const uint32_t nchan = bandpass.size();
  const uint32_t npol = bandpass[0].size();
  const uint32_t nchan_per_packet = weights_header.get_uint32("UDP_NCHAN");

  for (unsigned ichan=0; ichan<nchan; ichan++)
  {
    for (unsigned ipol=0; ipol<npol; ipol++)
    {
      float power = 0;
      for (unsigned isamp=0; isamp<nsamp; isamp++)
      {
        float value = get_float_value_for_channel_sample(ichan, nsamp, isamp, nbit);
        power += (value * value) + (value * value);
      }
      if (std::isnan(get_weight_for_channel(ichan, nchan_per_packet)))
      {
        power = 0;
      }
      float allowed_error = power / 100000; // NOLINT
      EXPECT_NEAR(power, bandpass[ichan][ipol], allowed_error);
    }
  }
}

TEST_P(DataUnpackerTest, test_unpack_performance) // NOLINT
{
  std::string param = GetParam();
  std::string data_file = param + "_data_header.txt";
  std::string weights_file = param + "_weights_header.txt";
  SPDLOG_DEBUG("data_file: {}", data_file);
  SPDLOG_DEBUG("weights_file: {}", weights_file);

  GeneratePackedData(data_file, weights_file);
  ska::pst::common::DataUnpacker unpacker;
  unpacker.configure(data_header, weights_header);

  // Recording the timestamp at the start of the loop
  auto beg = std::chrono::high_resolution_clock::now();

  const unsigned iterations = 10;
  for (int i=0; i<iterations; i++)
  {    
    // unpack data and weights
    std::vector<std::vector<std::vector<std::complex<float>>>>& unpacked = unpacker.unpack(&data[0], data.size(), &weights[0], weights.size());
  }

  // Taking a timestamp after the loop is finished
  auto end = std::chrono::high_resolution_clock::now();

  // Subtracting the end timestamp from the beginning
  // And we choose to receive the difference in microseconds
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
  auto duration_average = duration.count()/iterations;

  // Displaying the elapsed time
  SPDLOG_INFO("Data Size: {}", data.size());
  SPDLOG_INFO("Weights Size: {}", weights.size());
  SPDLOG_INFO("Elapsed Time (microseconds): {}", duration_average);
  // }
}

/*
TODO
- parse directory for all configuration files present in test/utils/data
- instansiate tests for each configuration file detected
*/
INSTANTIATE_TEST_SUITE_P(PerformanceTests, DataUnpackerTest, testing::Values(
"Low_AA0.5",
"Low_SB4",
"Mid_Band1_SB4",
"Mid_Band5a_SB4"
)); // NOLINT

} // namespace ska::pst::common::test
