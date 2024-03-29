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
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/FileBlockProducerTest.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

FileBlockProducerTest::FileBlockProducerTest()
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

FileBlockProducerTest::~FileBlockProducerTest()
{
  std::filesystem::remove(std::filesystem::path(file_name));
}

void FileBlockProducerTest::SetUp()
{
}

void FileBlockProducerTest::TearDown()
{
}

TEST_F(FileBlockProducerTest, test_get_header) // NOLINT
{
  SPDLOG_TRACE("ska::pst::common::test::FileBlockProducerTest::test_get_header construct from file_name={}", file_name);
  FileBlockProducer fr(file_name);
  SPDLOG_TRACE("ska::pst::common::test::FileBlockProducerTest::test_get_header get_header");
  EXPECT_EQ(fr.get_header().raw(), header.raw());
}

TEST_F(FileBlockProducerTest, test_open_bad_file) // NOLINT
{
  std::string bad_file_name = "/tmp/file/that/does/not/exist";
  SPDLOG_TRACE("ska::pst::common::test::FileBlockProducerTest::test_open_bad_file fr.open_file({})", bad_file_name);
  EXPECT_THROW(FileBlockProducer fr(bad_file_name), std::runtime_error); // NOLINT
}

TEST_F(FileBlockProducerTest, test_open_empty_file) // NOLINT
{
  std::string empty_file_name = "/tmp/FileBlockProducerTest_empty.dat";

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  int fd = open(empty_file_name.c_str(), flags, perms); // NOLINT
  close(fd);

  SPDLOG_TRACE("ska::pst::common::test::FileBlockProducerTest::test_open_empty_file fr.open_file({})", empty_file_name);
  EXPECT_THROW(FileBlockProducer fr(empty_file_name), std::runtime_error); // NOLINT

  std::filesystem::remove(std::filesystem::path(empty_file_name));
}

TEST_F(FileBlockProducerTest, test_bad_mmap) // NOLINT
{
  ska::pst::common::AsciiHeader bad_header(header);
  size_t bad_header_size = header_size + 1;
  bad_header.set("HDR_SIZE", bad_header_size);
  file_header.resize(bad_header_size);
  sprintf(&file_header[0], bad_header.raw().c_str(), header.raw().size()); // NOLINT

  std::string bad_file_name = "/tmp/FileBlockProducerTest_bad_mmap.dat";

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  int fd = open(bad_file_name.c_str(), flags, perms); // NOLINT
  write(fd, &file_header[0], bad_header_size);
  write(fd, &file_data[0], data_size);
  close(fd);

  EXPECT_THROW(FileBlockProducer fr(bad_file_name), std::runtime_error); // NOLINT

  std::filesystem::remove(std::filesystem::path(bad_file_name));
}

TEST_F(FileBlockProducerTest, test_next_block) // NOLINT
{
  FileBlockProducer fr(file_name);
  auto next = fr.next_block();
  EXPECT_EQ(next.size, data_size);

  auto file_data_ptr = reinterpret_cast<uint8_t *>(next.block);
  for (unsigned i=0; i<data_size; i++)
  {
    ASSERT_EQ(file_data_ptr[i], uint8_t(i % 256));  // NOLINT
  }
}

TEST_F(FileBlockProducerTest, test_read_more_data_than_available) // NOLINT
{
  FileBlockProducer fr(file_name);
  auto next = fr.next_block();
  next = fr.next_block();
  EXPECT_EQ(next.block, nullptr); // NOLINT
  EXPECT_EQ(next.size, 0);
}

} // namespace ska::pst::common::test
