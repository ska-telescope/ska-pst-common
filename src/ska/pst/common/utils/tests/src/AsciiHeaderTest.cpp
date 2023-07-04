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
#include <vector>

#include "ska/pst/common/utils/tests/AsciiHeaderTest.h"
#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/testutils/GtestMain.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

static constexpr int test_int_val = 123;
static constexpr float test_float_val = 1.2346789;
static constexpr double test_double_val = 1.23467890123456;

AsciiHeaderTest::AsciiHeaderTest()
  : ::testing::Test()
{
}

void AsciiHeaderTest::SetUp()
{
}

void AsciiHeaderTest::TearDown()
{
}

TEST_F(AsciiHeaderTest, test_construct_with_size) // NOLINT
{
  static constexpr size_t nbytes = 8192;
  AsciiHeader config(nbytes);
  EXPECT_EQ(config.get_header_size(), nbytes);
}

TEST_F(AsciiHeaderTest, test_construct_from_obj) // NOLINT
{
  AsciiHeader config1;
  config1.load_from_file(test_data_file("config.txt"));
  AsciiHeader config2(config1);
  EXPECT_EQ(config1.get_header_size(), config2.get_header_size());
  EXPECT_EQ(config1.raw(), config2.raw());
}

TEST_F(AsciiHeaderTest, test_clone_from_obj) // NOLINT
{
  AsciiHeader config1;
  config1.load_from_file(test_data_file("config.txt"));
  AsciiHeader config2;
  config2.clone(config1);
  EXPECT_EQ(config1.get_header_size(), config2.get_header_size());
  EXPECT_EQ(config1.raw(), config2.raw());
}

TEST_F(AsciiHeaderTest, test_clone_stream_from_obj) // NOLINT
{
  AsciiHeader config1;
  config1.set_val("KEY1", "VAL1");
  config1.set_val("KEY2_0", "VAL2");
  config1.set_val("KEY3_1", "VAL3");

  AsciiHeader config2;
  static constexpr unsigned stream = 0;
  config2.clone_stream(config1, stream);

  EXPECT_EQ(config1.get_header_size(), config2.get_header_size());
  EXPECT_EQ(config2.get_val("KEY1"), std::string("VAL1"));
  EXPECT_EQ(config2.get_val("KEY2"), std::string("VAL2"));
  EXPECT_THROW(config2.get_val("KEY3"), std::runtime_error); // NOLINT
}

TEST_F(AsciiHeaderTest, test_append_header) // NOLINT
{
  AsciiHeader config1, config2;
  config1.set_val("KEY1", "VAL1");
  config2.set_val("KEY2", "VAL2");
  config1.append_header(config2);
  EXPECT_EQ(config1.get_val("KEY1"), std::string("VAL1"));
  EXPECT_EQ(config1.get_val("KEY2"), std::string("VAL2"));
}

TEST_F(AsciiHeaderTest, test_raw) // NOLINT
{
  AsciiHeader config;
  config.set_val("KEY1", "VAL1");
  config.set_val("KEY2", "VAL2");

  static constexpr uint32_t padding1 = 1;
  static constexpr uint32_t padding2 = 2;
  static constexpr uint32_t padding6 = 6;
  config.set_key_padding(padding1);
  EXPECT_EQ(config.raw(), std::string("KEY1 VAL1\nKEY2 VAL2\n"));
  config.set_key_padding(padding2);
  EXPECT_EQ(config.raw(), std::string("KEY1 VAL1\nKEY2 VAL2\n"));
  config.set_key_padding(padding6);
  EXPECT_EQ(config.raw(), std::string("KEY1  VAL1\nKEY2  VAL2\n"));
}

TEST_F(AsciiHeaderTest, test_get_header_length) // NOLINT
{
  AsciiHeader config;
  config.set_val("KEY1", "VAL1");
  config.set_val("KEY2", "VAL2");
  config.set_key_padding(1);
  EXPECT_EQ(config.get_header_length(), strlen("KEY1 VAL1\nKEY2 VAL2\n"));
}

TEST_F(AsciiHeaderTest, test_load_from_file) // NOLINT
{
  AsciiHeader config;
  config.load_from_file(test_data_file("config.txt"));
  EXPECT_THROW(config.load_from_file(test_data_file("does_not_exist.txt")), std::runtime_error); // NOLINT
}

TEST_F(AsciiHeaderTest, test_load_from_str) // NOLINT
{
  AsciiHeader config;
  config.load_from_str("KEY1 VAL1\nKEY2  VAL2\n");
  config.set_key_padding(1);
  EXPECT_EQ(config.raw(), std::string("KEY1 VAL1\nKEY2 VAL2\n"));
}

TEST_F(AsciiHeaderTest, test_load_from_string) // NOLINT
{
  AsciiHeader config;
  config.load_from_string(std::string("KEY1 VAL1\nKEY2  VAL2\n"));
  config.set_key_padding(1);
  EXPECT_EQ(config.raw(), std::string("KEY1 VAL1\nKEY2 VAL2\n"));
}

TEST_F(AsciiHeaderTest, test_append_from_str) // NOLINT
{
  AsciiHeader config;
  config.set_val("KEY1", "VAL1");
  config.append_from_str("KEY2      VAL2\n");
  config.set_key_padding(1);
  EXPECT_EQ(config.raw(), std::string("KEY1 VAL1\nKEY2 VAL2\n"));
}

TEST_F(AsciiHeaderTest, test_del) // NOLINT
{
  AsciiHeader config;
  config.set_val("KEY1", "VAL1");
  config.set_val("KEY2", "VAL2");
  config.set_val("KEY3", "VAL3");
  config.del("KEY2");
  config.set_key_padding(1);
  EXPECT_EQ(config.raw(), std::string("KEY1 VAL1\nKEY3 VAL3\n"));
}

TEST_F(AsciiHeaderTest, test_has) // NOLINT
{
  AsciiHeader config;
  config.set_val("KEY1", "VAL1");
  EXPECT_EQ(config.has("KEY1"), true);
  EXPECT_EQ(config.has("KEY2"), false);
}

TEST_F(AsciiHeaderTest, test_get_size) // NOLINT
{
  EXPECT_EQ(AsciiHeader::get_size(test_data_file("config.txt").c_str()), 4096);
}

TEST_F(AsciiHeaderTest, get_header_size) // NOLINT
{
  AsciiHeader header;
  EXPECT_EQ(header.get_header_size(), ska::pst::common::AsciiHeader::default_header_size);
}

TEST_F(AsciiHeaderTest, get_val) // NOLINT
{
  AsciiHeader config;
  EXPECT_THROW(config.get_val("doesnotexist"), std::runtime_error); // NOLINT
}

TEST_F(AsciiHeaderTest, set_val_string) // NOLINT
{
  AsciiHeader header;
  std::string in("value");
  header.set_val("key", in);
  EXPECT_EQ(header.get_val("key"), in);
  header.set_val("key", in);
  EXPECT_EQ(header.get_val("key"), in);
}

TEST_F(AsciiHeaderTest, get_uint32) // NOLINT
{
  AsciiHeader header;
  static constexpr uint32_t in = 42;
  std::string strval = "42";
  header.set("key", in);
  EXPECT_EQ(header.get_uint32("key"), in);
  EXPECT_EQ(header.get_val("key"), strval);
}

TEST_F(AsciiHeaderTest, get_int32) // NOLINT
{
  AsciiHeader header;
  static constexpr int32_t in = -42;
  std::string strval = "-42";
  header.set("key", in);
  EXPECT_EQ(header.get_int32("key"), in);
  EXPECT_EQ(header.get_val("key"), strval);
}

TEST_F(AsciiHeaderTest, get_uint64) // NOLINT
{
  AsciiHeader header;
  static constexpr uint64_t in = 8589934592;
  std::string strval = "8589934592";
  header.set("key", in);
  EXPECT_EQ(header.get_uint64("key"), in);
  EXPECT_EQ(header.get_val("key"), strval);
}

TEST_F(AsciiHeaderTest, get_float) // NOLINT
{
  AsciiHeader header;
  static constexpr float in = 0.5;
  std::string strval = "0.5";
  header.set("key", in);
  EXPECT_EQ(header.get_float("key"), in);
  EXPECT_EQ(header.get_val("key"), strval);
}

TEST_F(AsciiHeaderTest, get_double) // NOLINT
{
  AsciiHeader header;
  static constexpr double in = 0.123456789;
  header.set("key", in);
  EXPECT_EQ(header.get_double("key"), in);
  std::istringstream iss(header.get_val("key"));
  double out{};
  iss >> out;
  EXPECT_EQ(in, out);
}

TEST_F(AsciiHeaderTest, test_header_get_keys) // NOLINT
{
  AsciiHeader config;
  config.set_val("KEY1", "VAL1");
  config.set_val("KEY2", "VAL2");
  config.set_val("KEY3", "VAL3");
  std::vector<std::string> keys = config.header_get_keys();
  EXPECT_EQ(keys.size(), 3);
  EXPECT_EQ(keys[0], "KEY1");
  EXPECT_EQ(keys[1], "KEY2");
  EXPECT_EQ(keys[2], "KEY3");
}

TEST_F(AsciiHeaderTest, test_key_padding) // NOLINT
{
  AsciiHeader config;
  static constexpr uint32_t padding = 123;
  config.set_key_padding(padding);
  EXPECT_EQ(config.get_key_padding(), padding);
}

TEST_F(AsciiHeaderTest, set_get_val_int64) // NOLINT
{
  AsciiHeader header;
  int64_t inval = test_int_val;
  int64_t outval = 0;
  header.set("key", inval);
  header.get("key", &outval);
  EXPECT_EQ(inval, outval);
  const AsciiHeader header2(header);
  header2.get("key", &outval);
  EXPECT_EQ(inval, outval);
}

TEST_F(AsciiHeaderTest, set_get_val_uint64) // NOLINT
{
  AsciiHeader header;
  uint64_t inval = test_int_val;
  uint64_t outval = 0;
  header.set("key", inval);
  header.get("key", &outval);
  EXPECT_EQ(inval, outval);
  const AsciiHeader header2(header);
  header2.get("key", &outval);
  EXPECT_EQ(inval, outval);
}

TEST_F(AsciiHeaderTest, set_get_val_int32) // NOLINT
{
  AsciiHeader header;
  int32_t inval = test_int_val;
  int32_t outval = 0;
  header.set("key", inval);
  header.get("key", &outval);
  EXPECT_EQ(inval, outval);
  const AsciiHeader header2(header);
  header2.get("key", &outval);
  EXPECT_EQ(inval, outval);
}

TEST_F(AsciiHeaderTest, set_get_val_uint32) // NOLINT
{
  AsciiHeader header;
  uint32_t inval = test_int_val;
  uint32_t outval = 0;
  header.set("key", inval);
  header.get("key", &outval);
  EXPECT_EQ(inval, outval);
  const AsciiHeader header2(header);
  header2.get("key", &outval);
  EXPECT_EQ(inval, outval);
}

TEST_F(AsciiHeaderTest, set_get_val_float) // NOLINT
{
  AsciiHeader header;
  float inval = test_float_val;
  float outval = NAN;
  header.set("key", inval);
  header.get("key", &outval);
  EXPECT_LT(fabsf(inval - outval), 0.00001);
  const AsciiHeader header2(header);
  header2.get("key", &outval);
  EXPECT_LT(fabsf(inval - outval), 0.00001);
}

TEST_F(AsciiHeaderTest, set_get_val_double) // NOLINT
{
  AsciiHeader header;
  double inval = test_double_val;
  double outval = NAN;
  header.set("key", inval);
  header.get("key", &outval);
  double diff = std::abs(inval - outval);
  EXPECT_LT(diff, 0.00000000001);
  const AsciiHeader header2(header);
  header2.get("key", &outval);
  EXPECT_LT(std::fabs(inval - outval), 0.00001f);
}

TEST_F(AsciiHeaderTest, get_with_bad_value) // NOLINT
{
  AsciiHeader header;
  std::string key = "KEY";
  std::string val = "VAL";
  header.set_val(key, val);
  int32_t val_i32 = 0;
  EXPECT_THROW(header.get(key, &val_i32), std::ios_base::failure); // NOLINT
  int64_t val_i64 = 0;
  EXPECT_THROW(header.get(key, &val_i64), std::ios_base::failure); // NOLINT
  uint64_t val_u64 = 0;
  EXPECT_THROW(header.get(key, &val_u64), std::ios_base::failure); // NOLINT
  uint32_t val_u32 = 0;
  EXPECT_THROW(header.get(key, &val_u32), std::ios_base::failure); // NOLINT
  float val_float = 0;
  EXPECT_THROW(header.get(key, &val_float), std::ios_base::failure); // NOLINT
  double val_double = 0;
  EXPECT_THROW(header.get(key, &val_double), std::ios_base::failure); // NOLINT
  std::string empty_val = "";
  EXPECT_THROW(header.set_val(key, empty_val), std::runtime_error); // NOLINT
}

TEST_F(AsciiHeaderTest, get_with_missing_key) // NOLINT
{
  AsciiHeader config1;
  config1.set_val("KEY", "1");
  int val_i32 = 0;
  EXPECT_THROW(config1.get("NOTKEY", &val_i32), std::runtime_error); // NOLINT

  const AsciiHeader config2(config1);
  config1.get("KEY", &val_i32);
  EXPECT_EQ(val_i32, 1);
  EXPECT_THROW(config2.get("NOTKEY", &val_i32), std::runtime_error); // NOLINT
}

TEST_F(AsciiHeaderTest, test_compute_bytes_per_second) // NOLINT
{
  AsciiHeader config;
  static constexpr unsigned nchan = 10;
  static constexpr unsigned npol = 2;
  static constexpr unsigned nbit = 16;
  static constexpr unsigned ndim = 2;
  config.set("NCHAN", nchan);
  config.set("NBIT", nbit);
  config.set("NPOL", npol);
  config.set("NDIM", ndim);
  unsigned expected_bits_per_sample = nchan * nbit * npol * ndim;
  EXPECT_EQ(config.compute_bits_per_sample(), expected_bits_per_sample);

  static constexpr double tsamp = 1.28;
  static constexpr double microseconds_per_second = 1000000;
  static constexpr double bits_per_byte = 8;
  config.set("TSAMP", tsamp);
  double expected_bytes_per_second = expected_bits_per_sample * microseconds_per_second / tsamp / bits_per_byte;
  EXPECT_EQ(config.compute_bytes_per_second(), expected_bytes_per_second);
}

} // namespace ska::pst::common::test
