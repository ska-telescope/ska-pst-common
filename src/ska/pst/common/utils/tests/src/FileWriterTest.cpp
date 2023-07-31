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
#include <initializer_list>
#include <filesystem>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/FileWriterTest.h"
#include "ska/pst/common/utils/FileReader.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

FileWriterTest::FileWriterTest()
    : ::testing::Test()
{
  header.load_from_file(test_data_file("data_scan_config.txt"));

  header_size = header.get_uint32("HDR_SIZE");
  file_header.resize(header_size);
  sprintf(&file_header[0], header.raw().c_str(), header.raw().size()); // NOLINT

  FileWriter tmp;
  posix_memalign(reinterpret_cast<void **>(&file_data), tmp.block_alignment(), data_size);
  auto file_data_ptr = reinterpret_cast<uint8_t *>(file_data);
  for (unsigned i=0; i<data_size; i++)
  {
    file_data_ptr[i] = uint8_t(i % 256); // NOLINT
  }
}

FileWriterTest::~FileWriterTest()
{
  if (std::filesystem::exists(std::filesystem::path(file_name)))
  {
    std::filesystem::remove(std::filesystem::path(file_name));
  }

  free(file_data); // NOLINT
}

void FileWriterTest::SetUp()
{
}

void FileWriterTest::TearDown()
{
}

TEST_F(FileWriterTest, test_default_constructor) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    FileWriter writer(use_o_direct);
    EXPECT_EQ(writer.is_file_open(), false);
  }
}

TEST_F(FileWriterTest, test_get_filename) // NOLINT
{
  FileWriter writer;
  std::string utc = "2023-07-31-13:41:23";
  static constexpr uint64_t obs_offset = 1234567890;
  static constexpr unsigned file_number = 256;

  auto filename = writer.get_filename(utc, obs_offset, file_number);
  std::string expect = utc + "_0000001234567890_000256.dada";
  EXPECT_EQ(filename, expect);
}

TEST_F(FileWriterTest, test_open_file) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    FileWriter writer(use_o_direct);
    writer.open_file(file_name);
    EXPECT_EQ(writer.is_file_open(), true);
    EXPECT_EQ(writer.get_header_bytes_written(), 0);
    EXPECT_EQ(writer.get_data_bytes_written(), 0);
  }
}

TEST_F(FileWriterTest, test_write_header) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header write_header");
    writer.write_header(header);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header header_bytes_written={}", writer.get_header_bytes_written());
    EXPECT_EQ(writer.get_header_bytes_written(), header_size);
  }
}

TEST_F(FileWriterTest, test_open_twice) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_twice use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_twice open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_twice incorrectly open again");
    EXPECT_THROW(writer.open_file(file_name), std::runtime_error); // NOLINT
  }
}

TEST_F(FileWriterTest, test_write_header_twice) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header_twice use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header_twice open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header_twice write_header");
    writer.write_header(header);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_header_twice incorrectly write_header again");
    EXPECT_THROW(writer.write_header(header), std::runtime_error); // NOLINT
  }
}

TEST_F(FileWriterTest, test_open_bad_file) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_twice use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    std::string bad_file_name = "/tmp/path/that/does/not/exist";
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_bad_file writer.open_file({})", bad_file_name);
    EXPECT_THROW(writer.open_file(bad_file_name), std::runtime_error); // NOLINT
  }
}

TEST_F(FileWriterTest, test_close_when_closed) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_when_closed use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_when_closed open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_when_closed writer.close_file()");
    writer.close_file();
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_when_closed writer.close_file()");
    EXPECT_THROW(writer.close_file(), std::runtime_error); // NOLINT
  }
}


TEST_F(FileWriterTest, test_bad_header_size) // NOLINT
{
  ska::pst::common::AsciiHeader bad_header;
  bad_header.clone(header);

  // set the HDR_SIZE to something less than the actual size of the header
  bad_header.set("HDR_SIZE", bad_header.raw().size()/2);

  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_bad_header_size use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_bad_header_size open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_bad_header_size write_header");
    EXPECT_THROW(writer.write_header(bad_header), std::runtime_error);
  }
}


TEST_F(FileWriterTest, test_large_header_size) // NOLINT
{
  ska::pst::common::AsciiHeader large_header;
  uint32_t large_header_size = header_size * 64; // NOLINT
  large_header.clone(header);
  large_header.set("HDR_SIZE", large_header_size);

  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_large_header_size use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_large_header_size open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_large_header_size write_header");
    EXPECT_EQ(writer.write_header(large_header), large_header_size);
  }
}

TEST_F(FileWriterTest, test_write_data_before_header) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data_before_header use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data_before_header open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data_before_header incorrectly write_data before write_header");
    EXPECT_THROW(writer.write_data(file_data, data_size), std::runtime_error);
  }
}

TEST_F(FileWriterTest, test_write_data) // NOLINT
{
  for (bool use_o_direct : {false,true})
  {
    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data use_o_direct={}", use_o_direct);
    FileWriter writer(use_o_direct);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data open filename={}", file_name);
    writer.open_file(file_name);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data write_header");
    writer.write_header(header);

    SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_data write_data");
    EXPECT_EQ(writer.write_data(file_data, data_size), data_size);

    FileReader reader (file_name);
    reader.read_header();
    EXPECT_EQ(reader.get_header().raw(), header.raw());

    std::vector<char> read_data(data_size);
    EXPECT_EQ(reader.read_data(&read_data[0], data_size), data_size);

    for (unsigned i=0; i<data_size; i++)
    {
      ASSERT_EQ(file_data[i], read_data[i]);  // NOLINT
    }
  }
}

TEST_F(FileWriterTest, test_write_unaligned_pointer) // NOLINT
{
  bool use_o_direct = true;
  
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_unaligned_pointer use_o_direct={}", use_o_direct);
  FileWriter writer(use_o_direct);

  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_unaligned_pointer open filename={}", file_name);
  writer.open_file(file_name);

  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_unaligned_pointer write_header");
  writer.write_header(header);

  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_unaligned_pointer write_data unaligned base address");
  EXPECT_THROW(writer.write_data(file_data+1, data_size/2), std::runtime_error); // NOLINT
}

TEST_F(FileWriterTest, test_write_less_than_block) // NOLINT
{
  bool use_o_direct = true;
  
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_less_than_block use_o_direct={}", use_o_direct);
  FileWriter writer(use_o_direct);

  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_less_than_block open filename={}", file_name);
  writer.open_file(file_name);

  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_less_than_block write_header");
  writer.write_header(header);

  size_t half_block = writer.block_alignment()/2;
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_write_less_than_block write_data half of logical block size");
  EXPECT_EQ(writer.write_data(file_data, half_block), half_block);
}

} // namespace ska::pst::common::test
