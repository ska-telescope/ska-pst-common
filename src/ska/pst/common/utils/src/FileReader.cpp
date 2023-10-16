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

#include <unistd.h>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <ostream>
#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ska/pst/common/utils/FileReader.h"

ska::pst::common::FileReader::FileReader(const std::string& file_path)
{
  open_file(file_path);
}

ska::pst::common::FileReader::~FileReader()
{
  SPDLOG_DEBUG("ska::pst::common::FileReader::~FileReader close_file()");
  if (fd > 0)
  {
    close_file();
  }
}

void ska::pst::common::FileReader::open_file(const std::string& file_path)
{
  if (fd >= 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileReader::open_file file is already opened fd={}", fd);
    throw std::runtime_error("ska::pst::common::FileReader::open_file file already opened");
  }

  int flags = O_RDONLY;

  SPDLOG_DEBUG("ska::pst::common::FileReader::open_file opening {}", file_path);
  fd = ::open(file_path.c_str(), flags); // NOLINT
  if (fd < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileReader::open_file failed to open file for reading");
    throw std::runtime_error("ska::pst::common::FileReader::open_file failed to open file for reading");
  }

  bytes_read_from_file = 0;

  // determine the size of the file in bytes
  file_size = std::filesystem::file_size(std::filesystem::path(file_path));
}

void ska::pst::common::FileReader::close_file()
{
  SPDLOG_DEBUG("ska::pst::common::FileReader::close_file fd={}", fd);
  if (fd < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileReader::close_file file is not open fd={}", fd);
    throw std::runtime_error("ska::pst::common::FileReader::close_file file not opened");
  }

  if (::close(fd) < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileReader::close_file ::close({}) failed: {}", fd, strerror(errno));
    throw std::runtime_error("ska::pst::common::FileReader::close_file failed to close file");
  }
  fd = -1;
}

auto ska::pst::common::FileReader::read_header() -> ssize_t
{
  static constexpr uint32_t default_header_size = ska::pst::common::AsciiHeader::default_header_size;
  std::vector<char> buffer;
  buffer.resize(default_header_size);

  // read the expected default header size, so that the exact header size can be determined
  ssize_t bytes_read = read(fd, &buffer[0], buffer.size());
  if (bytes_read != buffer.size())
  {
    SPDLOG_ERROR("ska::pst::common::FileReader::read_header attempted to read {} bytes from file, but only {} bytes were read", buffer.size(), bytes_read);
    throw std::runtime_error("ska::pst::common::FileReader::read_header failed to read header from file");
  }
  header.load_from_string(std::string(buffer.begin(), buffer.end()));

  uint32_t hdr_size = header.get_uint32("HDR_SIZE");

  // if the header size does not match the default, resize the buffer
  if (hdr_size != default_header_size)
  {
    buffer.resize(hdr_size);
    lseek(fd, 0, SEEK_SET);
    ssize_t bytes_read = read(fd, &buffer[0], buffer.size());
    if (bytes_read != buffer.size())
    {
      SPDLOG_ERROR("ska::pst::common::FileReader::read_header attempted to read {} bytes from file, but only {} bytes were read",
        buffer.size(), bytes_read);
      throw std::runtime_error("ska::pst::common::FileReader::read_header failed to read header from file");
    }
    header.load_from_string(std::string(buffer.begin(), buffer.end()));
  }

  if (header.has("OBS_OFFSET"))
  {
    obs_offset = header.get_uint32("OBS_OFFSET");
  }

  // increment the counter for bytes read
  bytes_read_from_file = hdr_size;
  return static_cast<ssize_t>(hdr_size);
}

auto ska::pst::common::FileReader::read_data(char * data_ptr, uint64_t bytes_to_read) -> ssize_t
{
  if ( data_ptr == nullptr )
  {
    SPDLOG_WARN("ska::pst::common::FileReader::read_data data_ptr is null");
    throw std::runtime_error("ska::pst::common::FileReader::read_data data_ptr is null");
  }

  size_t bytes_remaining = file_size - bytes_read_from_file;
  SPDLOG_TRACE("ska::pst::common::FileReader::read_data bytes_to_read={} bytes_remaining={}", bytes_to_read, bytes_remaining);

  if (bytes_to_read == 0)
  {
    return 0;
  }

  if (bytes_remaining < bytes_to_read)
  {
    bytes_to_read = bytes_remaining;
  }
  SPDLOG_DEBUG("ska::pst::common::FileReader::read_data reading bytes {} - {} of {}", bytes_read_from_file, bytes_read_from_file + bytes_to_read, file_size);

  ssize_t bytes_read = read(fd, reinterpret_cast<void *>(data_ptr), bytes_to_read);
  if (bytes_read != bytes_to_read)
  {
    SPDLOG_ERROR("ska::pst::common::FileReader::read_data bytes_read fewer bytes than expected requested={} actual={}", bytes_to_read, bytes_read);
    throw std::runtime_error("ska::pst::common::FileReader::read_data bytes_read fewer bytes than expected");
  }

  bytes_read_from_file += bytes_to_read;

  SPDLOG_TRACE("ska::pst::common::FileReader::read_data bytes_read {} bytes to file, total read {}", bytes_to_read, bytes_read_from_file);
  return bytes_read;
}
