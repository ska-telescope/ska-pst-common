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

#include "ska/pst/common/utils/BlockLoader.h"
#include "ska/pst/common/utils/FileReader.h"

#include <memory>

#ifndef __SKA_PST_COMMON_UTILS_FileBlockLoader_h
#define __SKA_PST_COMMON_UTILS_FileBlockLoader_h

namespace ska::pst::common {

  /**
   * @brief Interface used for reading blocks of data from a source
   *
   */
  class FileBlockLoader : public BlockLoader
  {
    public:

      /**
       * @brief Construct a new FileBlockLoader object.
       *
       * @param file_path path to the DADA file to open for reading
       */
      FileBlockLoader (const std::string& file_path);

      /**
       * @brief Destroy FileBlockLoader object
       *
       */
      ~FileBlockLoader ();

      /**
       * @brief Get the AsciiHeader that describes the DADA file contents
       *
       * @return const ska::pst::common::AsciiHeader& header of the DADA file
       */
      const ska::pst::common::AsciiHeader& get_header() const;

      /**
       * @brief Get the next block of data
       *
       * When first called, returns a pair containing
       * - the base address of the start of data in the memory-mapped DADA file
       * - the size of the file in bytes (minus the size of the header)
       * If called again, this function returns (nullptr, 0)
       */
      std::pair<char*,size_t> next_block();

    protected:

      //! the DADA file reader
      std::unique_ptr<ska::pst::common::FileReader> reader;

      //! the details of the entire block
      std::pair<char*,size_t> block_info;

      //! the details of the next block
      std::pair<char*,size_t> next_block_info;
  };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_UTILS_FileBlockLoader_h

