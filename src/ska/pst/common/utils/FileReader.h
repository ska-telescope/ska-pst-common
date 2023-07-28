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

#include <cstddef>
#include <filesystem>
#include <mutex>
#include <inttypes.h>

#include "ska/pst/common/utils/AsciiHeader.h"

#ifndef SKA_PST_COMMON_UTIL_FileReader_h
#define SKA_PST_COMMON_UTIL_FileReader_h

namespace ska::pst::common {

  /**
   * @brief Facilitates reading the data and weights files written by the \ref FileWriter class.
   *
   */
  class FileReader {

    public:

      /**
       * @brief Construct a new FileReader object.
       *
       * @param file_path path to file to open for reading
       */
      FileReader(const std::string& file_path);

      /**
       * @brief Destroy the FileReader object.
       *
       */
      ~FileReader();

      /**
       * @brief Open the file
       *
       * @param file_path path to file to open for reading
       */
      void open_file(const std::string& file_path);

      /**
       * @brief Close the currently opened file.
       *
       */
      void close_file();

      /**
       * @brief Read the ascii header from the currently open file
       *
       * @return ssize_t number of bytes read from file
       */
      ssize_t read_header();

      /**
       * @brief Read data from the currently opened file
       *
       * @param data_ptr pointer to the data to read from file
       * @param data_size number of bytes to read into the pointer
       * @return ssize_t number of bytes read from file
       */
      ssize_t read_data(char * data_ptr, uint64_t data_size);

      /**
       * @brief Get a const reference to the AsciiHeader that is populated when
       * \ref read_header is called
       *
       * @return const ska::pst::common::AsciiHeader& header of the opened file
       */
      const ska::pst::common::AsciiHeader& get_header() { return header; };

      /**
       * @brief Get the size of the file in bytes
       *
       * @return std::uintmax_t size of the file in bytes
       */
      std::uintmax_t get_file_size() { return file_size; };

      /**
       * @brief Set the file descriptor to the specified value
       *
       * @param _fd file descriptor to use to over-write the actual fd
       */
      void _set_fd(int _fd) { fd = _fd; };

      /**
       * @brief Return the file descriptor of the opened file.
       *
       * @return int file descriptor of the opened file
       */
      int _get_fd() { return fd; };

    private:

      //! the scan configuration parameters.
      ska::pst::common::AsciiHeader header;

      //! file descriptor of the currently opened file
      int fd{-1};

      //! total size of the file
      std::uintmax_t file_size{0};

      //! total number of bytes read from file
      uint64_t bytes_read_from_file{0};

  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTIL_FileReader_h
