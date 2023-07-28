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
#include <filesystem>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/FileReaderTest.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

FileReaderTest::FileReaderTest()
    : ::testing::Test()
{
  header.load_from_file(test_data_file("data_scan_config.txt"));

  header_size = header.get_uint32("HDR_SIZE");
  file_header.resize(header_size);
  sprintf(&file_header[0], header.raw().c_str(), header.raw().size()); // NOLINT

  file_data.resize(data_size);
  auto file_data_ptr = reinterpret_cast<uint8_t *>(&file_data[0]);
  for (unsigned i=0; i<data_size; i++)
  {
    file_data_ptr[i] = uint8_t(i % 256); // NOLINT
  }

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  int fd = open(file_name.c_str(), flags, perms); // NOLINT
  write(fd, &file_header[0], header_size);
  write(fd, &file_data[0], data_size);
  close(fd);
}

FileReaderTest::~FileReaderTest()
{
  std::filesystem::remove(std::filesystem::path(file_name));
}

void FileReaderTest::SetUp()
{
}

void FileReaderTest::TearDown()
{
}

TEST_F(FileReaderTest, test_default_constructor) // NOLINT
{
  FileReader fr(file_name);
  EXPECT_EQ(fr.get_file_size(), header_size + data_size);
}

TEST_F(FileReaderTest, test_read_header) // NOLINT
{
  FileReader fr(file_name);
  fr.read_header();
  EXPECT_EQ(fr.get_header().raw(), header.raw());
}

TEST_F(FileReaderTest, test_open_two_files) // NOLINT
{
  FileReader fr(file_name);
  EXPECT_THROW(fr.open_file(file_name), std::runtime_error); // NOLINT
}

TEST_F(FileReaderTest, test_open_bad_file) // NOLINT
{
  FileReader fr(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileReaderTest::test_open_bad_file fr.close_file()");
  fr.close_file();

  std::string bad_file_name = "/tmp/file/that/does/not/exist";
  SPDLOG_TRACE("ska::pst::common::test::FileReaderTest::test_open_bad_file fr.open_file({})", bad_file_name);
  EXPECT_THROW(fr.open_file(bad_file_name), std::runtime_error); // NOLINT
}

TEST_F(FileReaderTest, test_close_when_closed) // NOLINT
{
  FileReader fr(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileReaderTest::test_close_when_closed fr.close_file()");
  fr.close_file();
  SPDLOG_TRACE("ska::pst::common::test::FileReaderTest::test_close_when_closed fr.close_file()");
  EXPECT_THROW(fr.close_file(), std::runtime_error); // NOLINT
}

TEST_F(FileReaderTest, test_close_with_bad_fd) // NOLINT
{
  FileReader fr(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileReaderTest::test_close_with_bad_fd fr.close_file()");
  int real_fd = fr._get_fd();
  fr._set_fd(real_fd + 1);
  EXPECT_THROW(fr.close_file(), std::runtime_error); // NOLINT
  fr._set_fd(real_fd);
  EXPECT_NO_THROW(fr.close_file()); // NOLINT
}

TEST_F(FileReaderTest, test_open_tiny_file) // NOLINT
{
  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  std::string tiny_file = "/tmp/FileReaderTestTinyFile.dada";
  int fd = open(tiny_file.c_str(), flags, perms);  // NOLINT
  write(fd, &file_header[0], header_size / 2);
  close(fd);

  FileReader fr(tiny_file);
  EXPECT_THROW(fr.read_header(), std::runtime_error); // NOLINT

  std::filesystem::remove(std::filesystem::path(tiny_file));
}

TEST_F(FileReaderTest, test_bad_header_size) // NOLINT
{
  ska::pst::common::AsciiHeader bad_header;
  bad_header.clone(header);
  bad_header.set("HDR_SIZE", header_size * 2);

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  std::string bad_header_file = "/tmp/FileReaderTestBadHeader.dada";
  int fd = open(bad_header_file.c_str(), flags, perms);  // NOLINT
  std::vector<char> bad_file_header(header_size);
  sprintf(&bad_file_header[0], bad_header.raw().c_str(), bad_header.raw().size()); // NOLINT
  write(fd, &bad_file_header[0], header_size);
  close(fd);

  FileReader fr(bad_header_file);
  EXPECT_THROW(fr.read_header(), std::runtime_error); // NOLINT

  std::filesystem::remove(std::filesystem::path(bad_header_file));
}

TEST_F(FileReaderTest, test_large_header_size) // NOLINT
{
  ska::pst::common::AsciiHeader large_header;
  uint32_t large_header_size = header_size * 2;
  large_header.clone(header);
  large_header.set("HDR_SIZE", large_header_size);

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  std::string large_header_file = "/tmp/FileReaderTestLargeHeader.dada";
  int fd = ::open(large_header_file.c_str(), flags, perms);  // NOLINT
  std::vector<char> large_file_header(large_header_size);
  sprintf(&large_file_header[0], large_header.raw().c_str(), large_header.raw().size()); // NOLINT
  ::write(fd, &large_file_header[0], large_header_size);
  close(fd);

  FileReader fr(large_header_file);
  EXPECT_NO_THROW(fr.read_header()); // NOLINT

  std::filesystem::remove(std::filesystem::path(large_header_file));
}

TEST_F(FileReaderTest, test_read_data) // NOLINT
{
  FileReader fr(file_name);
  fr.read_header();
  std::vector<char> _data(data_size);
  EXPECT_EQ(fr.read_data(&_data[0], data_size), data_size);

  auto file_data_ptr = reinterpret_cast<uint8_t *>(&_data[0]);
  for (unsigned i=0; i<data_size; i++)
  {
    ASSERT_EQ(file_data_ptr[i], uint8_t(i % 256));  // NOLINT
  }
}

TEST_F(FileReaderTest, test_read_more_data_than_available) // NOLINT
{
  FileReader fr(file_name);
  fr.read_header();
  uint64_t larger_data_size = data_size * 2;
  std::vector<char> _data(larger_data_size);
  SPDLOG_TRACE("ska::pst::common::test::FileReaderTest::test_read_more_data_than_available data_size={} larger_data_size={}", larger_data_size);
  EXPECT_EQ(fr.read_data(&_data[0], larger_data_size), data_size); // NOLINT
}

TEST_F(FileReaderTest, test_read_data_with_bad_fd) // NOLINT
{
  FileReader fr(file_name);
  fr.read_header();
  std::vector<char> _data(data_size);
  int real_fd = fr._get_fd();
  fr._set_fd(real_fd + 1);
  EXPECT_THROW(fr.read_data(&_data[0], data_size), std::runtime_error); // NOLINT
  fr._set_fd(real_fd);
  EXPECT_NO_THROW(fr.read_data(&_data[0], data_size)); // NOLINT
}

} // namespace ska::pst::common::test
