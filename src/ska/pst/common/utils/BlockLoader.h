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

#include <string>

#include "ska/pst/common/utils/AsciiHeader.h"

#ifndef __SKA_PST_COMMON_UTILS_BlockLoader_h
#define __SKA_PST_COMMON_UTILS_BlockLoader_h

namespace ska::pst::common {

  /**
   * @brief Interface used for reading blocks of data from a source
   *
   */
  class BlockLoader
  {
    public:

      /**
       * @brief Stores the base address and size, in bytes, of a block of data
       *
       */
      class Block
      {
        public:

          Block (char* _block=nullptr, size_t _size=0) : block(_block), size(_size) {}

          //! pointer to the block
          char* block;

          //! the size, in bytes, of the block
          size_t size;
      };

      /**
       * @brief Virtual destructor required for interfaces
       *
       */
      virtual ~BlockLoader () = default;

      /**
       * @brief Get the AsciiHeader that describes the block stream
       *
       * @return const ska::pst::common::AsciiHeader& header of the data
       */
      virtual const ska::pst::common::AsciiHeader& get_header() const = 0;

      /**
       * @brief Get the next block of data
       *
       * This returns a pair that contains the pointer to the next block of data
       * and the size, in bytes, of that block.
       * At the end of the block stream, this function returns (nullptr, 0)
       *
       * @return (char* address of block, size_t bytes in block)
       */
      virtual Block next_block() = 0;
  };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_UTILS_BlockLoader_h

