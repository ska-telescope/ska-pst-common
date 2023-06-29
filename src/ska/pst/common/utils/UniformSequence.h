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

#include <inttypes.h>
#include <random>

#include "ska/pst/common/utils/AsciiHeader.h"

#ifndef SKA_PST_COMMON_UTILS_UniformSequence_h
#define SKA_PST_COMMON_UTILS_UniformSequence_h

namespace ska::pst::common {

  /**
   * @brief Generates a uniform sequence of char values.
   *
   */
  class UniformSequence {

    public:

      /**
       * @brief Construct a new Uniform Sequence object
       *
       */
      UniformSequence(const char value);

      /**
       * @brief Destroy the Uniform Sequence object
       *
       */
      ~UniformSequence() = default;

      /**
       * @brief Configure the Uniform Sequence using the meta data present in the
       * AsciiHeader.
       *
       * @param header header containing a UTC_START and OBS_OFFSET key/val pair
       */
      void configure(const ska::pst::common::AsciiHeader& header);

      /**
       * @brief Reset the internal state of the Uniform Sequence.
       * The next call to generate or validate will behave as per the first call to these functions.
       *
       */
      void reset();

      /**
       * @brief Generate a uniform data sequence.
       *
       * @param buffer pointer to memory to which the uniform sequence should be written
       * @param bufsz number of bytes to write to the buffer.
       */
      void generate(char * buffer, uint64_t bufsz);

      /**
       * @brief Generate a uniform data sequence written to the provided buffer in blocks
       *
       * @param buffer buffer to write the uniform sequence to
       * @param bufsz size of the buffer in bytes
       * @param block_offset offset from the start of the buffer for the first block
       * @param block_size size of each block of uniform data to write in bytes
       * @param block_stride separate between each block of uniform data in bytes
       */
      void generate_block(char * buffer, uint64_t bufsz, uint64_t block_offset, uint64_t block_size, uint64_t block_stride);

      /**
       * @brief Compare contents of buffer to expected random sequence
       *
       * @param buffer pointer to buffer containing samples to be validated
       * @param bufsz size of the buffer to be validated
       * @return true if all samples are valid
       * @return false if any samples are invalid
       */
      bool validate(char * buffer, uint64_t bufsz);

      /**
       * @brief Validate the uniform data sequence written to the provided buffer in blocks
       *
       * @param buffer pointer to buffer containing samples to be validated
       * @param bufsz size of the buffer to be validated
       * @param block_offset offset from the start of the buffer for the first block
       * @param block_size size of each block of uniform data to write in bytes
       * @param block_stride separate between each block of uniform data in bytes
       * @return true if all samples are valid
       * @return false if any samples are invalid
       */
      bool validate_block(char * buffer, uint64_t bufsz, uint64_t block_offset, uint64_t block_size, uint64_t block_stride);

      //! verbosity flag used during debugging
      bool verbose{false};

    private:

      //! char value to write to all bytes
      const char uniform_value;
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_UniformSequence_h
