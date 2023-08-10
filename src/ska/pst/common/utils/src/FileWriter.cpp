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

#include "ska/pst/common/utils/FileWriter.h"

// casts an unsigned integer to an integer and throws an out-of-range exception if the unsigned argument is greater than the maximum possible signed value
template<typename T>
auto safe_signed_cast (const T& arg) -> std::make_signed_t<T>
{
  if (arg > std::numeric_limits<std::make_signed_t<T>>::max())
  {
    throw std::out_of_range("safe_cast_to_signed");
  }
  return static_cast<std::make_signed_t<T>>(arg);
}

ska::pst::common::FileWriter::FileWriter(bool use_o_direct) :
  o_direct(use_o_direct), flags(O_WRONLY | O_CREAT | O_TRUNC)
{
  SPDLOG_TRACE("ska::pst::common::FileWriter::FileWriter");
}

ska::pst::common::FileWriter::~FileWriter()
{
  SPDLOG_TRACE("ska::pst::common::FileWriter::~FileWriter");
  deconfigure();
}

void ska::pst::common::FileWriter::check_block_size(uint64_t block_size) const
{
  if (block_size == 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::check_block_size block_size is zero");
    throw std::runtime_error("ska::pst::common::FileWriter::check_block_size block_size is zero");
  }

  if (o_direct && (block_size % o_direct_alignment != 0))
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::check_block_size block_size={} must be a multiple of {} bytes when O_DIRECT is enabled", block_size, o_direct_alignment);
    throw std::runtime_error("ska::pst::common::FileWriter::check_block_size block_size is not a multiple of logical block size");
  }
}

void ska::pst::common::FileWriter::configure(uint64_t _header_bufsz)
{
  check_block_size (_header_bufsz);

  if (header_bufsz > 0 && _header_bufsz > header_bufsz)
  {
    // the currently allocated buffer is not large enough
    deconfigure();
  }

  if (!header_buffer)
  {
    SPDLOG_DEBUG("ska::pst::common::FileWriter::configure posix_memalign(header_buffer, {}, {})", o_direct_alignment, _header_bufsz);
    posix_memalign(reinterpret_cast<void **>(&header_buffer), o_direct_alignment, _header_bufsz);
    memset(header_buffer, 0, _header_bufsz);
    header_bufsz = _header_bufsz;
  }
}

void ska::pst::common::FileWriter::deconfigure()
{
  if (header_buffer)
  {
    SPDLOG_DEBUG("ska::pst::common::FileWriter::deconfigure free(header_buffer)");
    free(header_buffer); // NOLINT
  }
  header_buffer = nullptr;
  header_bufsz = 0;
}

auto ska::pst::common::FileWriter::get_filename(const std::string& utc_start, uint64_t obs_offset, unsigned file_number) -> std::filesystem::path
{
  std::ostringstream oss;
  oss << utc_start << "_" << std::setfill('0') << std::setw(obs_offset_width) << obs_offset
      << "_" << std::setw(file_number_width) << file_number << ".dada";
  return oss.str();
}

auto ska::pst::common::FileWriter::is_file_open() -> bool
{
  return file_open;
}

void ska::pst::common::FileWriter::open_file(const std::filesystem::path& new_file)
{
  if (file_open)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::open_file already open");
    throw std::runtime_error("ska::pst::common::FileWriter::open_file already open");
  }

  int perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
  if (o_direct)
  {
    flags |= O_DIRECT;
    perms |= S_IWUSR;
  }

  SPDLOG_DEBUG("ska::pst::common::FileWriter::open_file opening {}", new_file.generic_string());
  fd = open(new_file.generic_string().c_str(), flags, perms); // NOLINT
  if (fd < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::open_file failed to open {} for writing.", new_file.generic_string());
    throw std::runtime_error("Unable to open file " + new_file.generic_string());
  }
  file_open = true;
  opened_file = new_file;
  header_bytes_written = 0;
  data_bytes_written = 0;
}

void ska::pst::common::FileWriter::reopen_file()
{
  // save the header and data bytes written to file
  uint64_t current_data_bytes_written = data_bytes_written;
  uint64_t current_header_bytes_written = header_bytes_written;
  SPDLOG_DEBUG("ska::pst::common::FileWriter::reopen_file current_data_bytes_written={} current_header_bytes_written={}", current_data_bytes_written, current_header_bytes_written);

  SPDLOG_DEBUG("ska::pst::common::FileWriter::reopen_file close_file()");
  close_file();

  SPDLOG_DEBUG("ska::pst::common::FileWriter::reopen_file open_file without O_TRUNC");

  flags = O_WRONLY | O_CREAT;
  open_file(opened_file);

  // restore the usual O_TRUNC behaviour
  flags = O_WRONLY | O_CREAT | O_TRUNC;

  data_bytes_written = current_data_bytes_written;
  header_bytes_written = current_header_bytes_written;

  SPDLOG_DEBUG("ska::pst::common::FileWriter::reopen_file lseek({}, 0, SEEK_END)", fd);
  auto offset = lseek(fd, 0, SEEK_END);

  if (offset != data_bytes_written + header_bytes_written)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::reopen_file lseek returns offset={} != bytes_written={}", offset, data_bytes_written + header_bytes_written);
    throw std::runtime_error("ska::pst::common::FileWriter::reopen_file lseek returns offset != bytes_written");
  }
}

void ska::pst::common::FileWriter::close_file()
{
  if (!file_open)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::close_file not open");
    throw std::runtime_error("ska::pst::common::FileWriter::close_file not open");
  }

  SPDLOG_DEBUG("ska::pst::common::FileWriter::close_file fd={}", fd);
  if (::close(fd) < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::close_file ::close({}) failed: {}", fd, strerror(errno));
  }
  fd = -1;
  file_open = false;
}

auto ska::pst::common::FileWriter::write_header(const ska::pst::common::AsciiHeader& header) -> ssize_t
{
  SPDLOG_DEBUG("ska::pst::common::FileWriter::write_header()");

  if (header_bytes_written > 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_header header bytes already written={}", header_bytes_written);
    throw std::runtime_error("ska::pst::common::FileWriter::write_header header already written");
  }

  // ensure that the header buffer is allocated (and large enough)
  auto header_size = header.get_uint32("HDR_SIZE");
  configure (header_size);

  if (header_bufsz < header.raw().length())
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_header header_bufsz={} smaller than header.raw.length()={} (HDR_SIZE={})", header_bufsz, header.raw().length(), header_size);
    throw std::runtime_error("ska::pst::common::FileWriter::write_header header_bufsz smaller than header.raw.length after calling configure()");
  }

  sprintf(header_buffer, header.raw().c_str(), header.raw().length()); // NOLINT

  ssize_t wrote = write(fd, header_buffer, header_bufsz);
  if (wrote < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_header write({}, {}, {}) failed: {}", fd, reinterpret_cast<void *>(header_buffer), header_bufsz, strerror(errno));
    throw std::runtime_error("ska::pst::common::FileWriter::write_header could not write header to file");
  }

  if (wrote != header_bufsz)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_header wrote fewer bytes than expected requested={} actual={}", header_bufsz, wrote);
    throw std::runtime_error("ska::pst::common::FileWriter::write_header wrote fewer bytes than expected");
  }

  if (!o_direct)
  {
    // This won't block, but will start writeout asynchronously
    sync_file_range(fd, 0, safe_signed_cast(header_bufsz), SYNC_FILE_RANGE_WRITE);
  }
  header_bytes_written = header_bufsz;
  return wrote;
}

auto ska::pst::common::FileWriter::write_data(char * data_ptr, uint64_t bytes_to_write) -> ssize_t
{
  SPDLOG_DEBUG("ska::pst::common::FileWriter::write_data writing {} bytes to file", bytes_to_write);

  if (header_bytes_written == 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_data header not written");
    throw std::runtime_error("ska::pst::common::FileWriter::write_data header not written");
  }

  if (o_direct && (bytes_to_write % o_direct_alignment != 0))
  {
    SPDLOG_WARN("ska::pst::common::FileWriter::write_data bytes_to_write={} not a multiple of {} bytes", bytes_to_write, o_direct_alignment);

    // disable o_direct flag and re-open the file
    o_direct = false;
    reopen_file();
  }

  ssize_t wrote = write(fd, reinterpret_cast<void *>(data_ptr), bytes_to_write);

  if (wrote < 0)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_data write({}, {}, {}) failed: {}", fd, reinterpret_cast<void *>(data_ptr), bytes_to_write, strerror(errno));
    throw std::runtime_error("ska::pst::common::FileWriter::write_data could not write data to file");
  }

  if (wrote != bytes_to_write)
  {
    SPDLOG_ERROR("ska::pst::common::FileWriter::write_data wrote fewer bytes than expected requested={} actual={}", bytes_to_write, wrote);
    throw std::runtime_error("ska::pst::common::FileWriter::write_data wrote fewer bytes than expected");
  }

  if (!o_direct)
  {
    auto offset = safe_signed_cast(header_bytes_written + data_bytes_written);
    auto nbytes = safe_signed_cast(bytes_to_write);

    // This won't block, but will start writeout asynchronously
    sync_file_range(fd, offset, nbytes, SYNC_FILE_RANGE_WRITE);

    // This does a blocking write-and-wait on any old ranges
    if (data_bytes_written > 0)
    {
      sync_file_range(fd, offset, nbytes, SYNC_FILE_RANGE_WAIT_BEFORE|SYNC_FILE_RANGE_WRITE|SYNC_FILE_RANGE_WAIT_AFTER);
    }
  }

  data_bytes_written += wrote;

  SPDLOG_DEBUG("ska::pst::common::FileWriter::write_data wrote {} bytes to file, total written {}", bytes_to_write, data_bytes_written);
  return wrote;
}
