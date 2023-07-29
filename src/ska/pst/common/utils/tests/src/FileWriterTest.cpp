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
}

FileWriterTest::~FileWriterTest()
{
  std::filesystem::remove(std::filesystem::path(file_name));
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

#if 0

TEST_F(FileWriterTest, test_open_two_files) // NOLINT
{
  FileWriter writer(file_name);
  EXPECT_THROW(writer.open_file(file_name), std::runtime_error); // NOLINT
}

TEST_F(FileWriterTest, test_open_bad_file) // NOLINT
{
  FileWriter writer(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_bad_file writer.close_file()");
  writer.close_file();

  std::string bad_file_name = "/tmp/file/that/does/not/exist";
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_open_bad_file writer.open_file({})", bad_file_name);
  EXPECT_THROW(writer.open_file(bad_file_name), std::runtime_error); // NOLINT
}

TEST_F(FileWriterTest, test_close_when_closed) // NOLINT
{
  FileWriter writer(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_when_closed writer.close_file()");
  writer.close_file();
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_when_closed writer.close_file()");
  EXPECT_THROW(writer.close_file(), std::runtime_error); // NOLINT
}

TEST_F(FileWriterTest, test_close_with_bad_fd) // NOLINT
{
  FileWriter writer(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_close_with_bad_fd writer.close_file()");
  int real_fd = writer._get_fd();
  writer._set_fd(real_fd + 1);
  EXPECT_THROW(writer.close_file(), std::runtime_error); // NOLINT
  writer._set_fd(real_fd);
  EXPECT_NO_THROW(writer.close_file()); // NOLINT
}

TEST_F(FileWriterTest, test_open_tiny_file) // NOLINT
{
  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  std::string tiny_file = "/tmp/FileWriterTestTinyFile.dada";
  int fd = open(tiny_file.c_str(), flags, perms);  // NOLINT
  write(fd, &file_header[0], header_size / 2);
  close(fd);

  FileWriter writer(tiny_file);
  EXPECT_THROW(writer.read_header(), std::runtime_error); // NOLINT

  std::filesystem::remove(std::filesystem::path(tiny_file));
}

TEST_F(FileWriterTest, test_bad_header_size) // NOLINT
{
  ska::pst::common::AsciiHeader bad_header;
  bad_header.clone(header);
  bad_header.set("HDR_SIZE", header_size * 2);

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  std::string bad_header_file = "/tmp/FileWriterTestBadHeader.dada";
  int fd = open(bad_header_file.c_str(), flags, perms);  // NOLINT
  std::vector<char> bad_file_header(header_size);
  sprintf(&bad_file_header[0], bad_header.raw().c_str(), bad_header.raw().size()); // NOLINT
  write(fd, &bad_file_header[0], header_size);
  close(fd);

  FileWriter writer(bad_header_file);
  EXPECT_THROW(writer.read_header(), std::runtime_error); // NOLINT

  std::filesystem::remove(std::filesystem::path(bad_header_file));
}

TEST_F(FileWriterTest, test_large_header_size) // NOLINT
{
  ska::pst::common::AsciiHeader large_header;
  uint32_t large_header_size = header_size * 2;
  large_header.clone(header);
  large_header.set("HDR_SIZE", large_header_size);

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  std::string large_header_file = "/tmp/FileWriterTestLargeHeader.dada";
  int fd = ::open(large_header_file.c_str(), flags, perms);  // NOLINT
  std::vector<char> large_file_header(large_header_size);
  sprintf(&large_file_header[0], large_header.raw().c_str(), large_header.raw().size()); // NOLINT
  ::write(fd, &large_file_header[0], large_header_size);
  close(fd);

  FileWriter writer(large_header_file);
  EXPECT_NO_THROW(writer.read_header()); // NOLINT

  std::filesystem::remove(std::filesystem::path(large_header_file));
}

TEST_F(FileWriterTest, test_read_data) // NOLINT
{
  FileWriter writer(file_name);
  writer.read_header();
  std::vector<char> _data(data_size);
  EXPECT_EQ(writer.read_data(&_data[0], data_size), data_size);

  auto file_data_ptr = reinterpret_cast<uint8_t *>(&_data[0]);
  for (unsigned i=0; i<data_size; i++)
  {
    ASSERT_EQ(file_data_ptr[i], uint8_t(i % 256));  // NOLINT
  }
}

TEST_F(FileWriterTest, test_read_more_data_than_available) // NOLINT
{
  FileWriter writer(file_name);
  writer.read_header();
  uint64_t larger_data_size = data_size * 2;
  std::vector<char> _data(larger_data_size);
  SPDLOG_TRACE("ska::pst::common::test::FileWriterTest::test_read_more_data_than_available data_size={} larger_data_size={}", larger_data_size);
  EXPECT_EQ(writer.read_data(&_data[0], larger_data_size), data_size); // NOLINT
}

TEST_F(FileWriterTest, test_read_data_with_bad_fd) // NOLINT
{
  FileWriter writer(file_name);
  writer.read_header();
  std::vector<char> _data(data_size);
  int real_fd = writer._get_fd();
  writer._set_fd(real_fd + 1);
  EXPECT_THROW(writer.read_data(&_data[0], data_size), std::runtime_error); // NOLINT
  writer._set_fd(real_fd);
  EXPECT_NO_THROW(writer.read_data(&_data[0], data_size)); // NOLINT
}

#endif // 0

} // namespace ska::pst::common::test
