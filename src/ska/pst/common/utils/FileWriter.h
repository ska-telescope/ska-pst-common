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

#ifndef SKA_PST_COMMON_UTILS_FileWriter_h
#define SKA_PST_COMMON_UTILS_FileWriter_h

namespace ska::pst::common {

  /**
   * @brief The File Writer provides a simple API to write PSRDADA compliant files to the
   * file system.
   *
   */
  class FileWriter {

    public:

      /**
       * @brief Construct a new FileWriter object that supports O_DIRECT file access.
       *
       * @param use_o_direct Flag to enable the O_DIRECT option.
       */
      FileWriter(bool use_o_direct);

      /**
       * @brief Destroy the FileWriter object
       *
       */
      ~FileWriter();

      /**
       * @brief Configure the file writer, allocating an internal buffer that will be
       * used to write the header to the file.
       *
       * @param header_bufsz size of a header buffer element in bytes
       */
      void configure(uint64_t header_bufsz);

      /**
       * @brief Deconfigure the file writer, releasing the internal buffer.
       *
       */
      void deconfigure();

      /**
       * @brief Return a boolean describing if a file is currently open for writing.
       *
       * @return true a file is currently open for writing
       * @return false a files is not open for writing
       */
      bool is_file_open();

      /**
       * @brief Open the output file specified
       *
       * @param new_file full path to the file to be opened
       */
      void open_file(const std::filesystem::path& new_file);

      /**
       * @brief Write the header to the currently opened file
       *
       * @param header AsciiHeader to write to to the currently open file
       * @return ssize_t number of bytes written to the file
       */
      ssize_t write_header(const ska::pst::common::AsciiHeader& header);

      /**
       * @brief Write data to the currently opened file
       *
       * @param data_ptr pointer to the data to write
       * @param data_size number of bytes to write to the file
       * @return ssize_t number of bytes written to the file
       */
      ssize_t write_data(char * data_ptr, uint64_t data_size);

      /**
       * @brief Close the currently opened file.
       *
       */
      void close_file();

      /**
       * @brief Get the number of header bytes written to the current file
       *
       * @return uint64_t the number of header bytes written
       */
      uint64_t get_header_bytes_written() { return header_bytes_written; };

      /**
       * @brief Get the number of data bytes written to the current file
       *
       * @return uint64_t the number data bytes written to the current file
       */
      uint64_t get_data_bytes_written() { return data_bytes_written; };

      /**
       * @brief Get the filename for the specified scan_id, obs_offset and file_number.
       * Output filename will be structured as [UTC_START]_[OBS_OFFSET]_[FILE_NUMBER].dada
       *
       * @param utc_start the UTC_START timestamp of the file to be written
       * @param obs_offset offset in bytes from the start of the observation
       * @param file_number file number in the sequence
       * @return std::filesystem::path full path to the output file
       */
      static std::filesystem::path get_filename(const std::string& utc_start, uint64_t obs_offset, unsigned file_number);

    private:

      /**
       * @brief Close and re-open the current file with the O_DIRECT flag disabled.
       *
       */
      void reopen_file();

      //! path the currently opened file
      std::filesystem::path opened_file;

      //! the header parameters read from the data block
      ska::pst::common::AsciiHeader header;

      //! file descriptor of the currently opened file
      int fd{-1};

      //! local buffer of page-aligned memory used to write the AsciiHeader to O_DIRECT opened file handles
      char* header_buffer{nullptr};

      //! size of the header ring buffer elements in bytes
      uint64_t header_bufsz{0};

      //! path of the currently opened file
      std::filesystem::path current_file;

      //! boolean flag indicating if an output file is currently open
      bool file_open = false;

      //! flag which instructs open_file to enable O_DIRECT flag when opening files
      bool o_direct{false};

      //! alignment for I/O operations on O-DIRECT buffers
      const uint32_t o_direct_alignment{512};

      uint64_t header_bytes_written{0};

      uint64_t data_bytes_written{0};

      //! number of 0-padded characters in obs_offset portion of file name
      static const uint32_t obs_offset_width{16};

      //! number of 0-padded characters in file_number portion of file name
      static const uint32_t file_number_width{6};

  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_FileWriter_h
