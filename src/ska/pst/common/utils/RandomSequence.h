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

#include <cinttypes>
#include <random>

#include "ska/pst/common/utils/AsciiHeader.h"

#ifndef SKA_PST_COMMON_UTILS_RandomSequence_h
#define SKA_PST_COMMON_UTILS_RandomSequence_h

namespace ska::pst::common {

  /**
   * @brief Generates a sequence of randomly distributed unsigned 8-bit integer values.
   * The configuration of the distribution is controlled by header supplied in the
   * configure method, through the following parameters:
   *   UTC_START: sees the random number generator from which the sequence is generated
   *   OBS_OFFSET: byte offset into the random sequence
   *
   */
  class RandomSequence {

    public:

      /**
       * @brief Construct a new Random Sequence object
       *
       */
      RandomSequence() = default;

      /**
       * @brief Destroy the Random Sequence object
       *
       */
      ~RandomSequence() = default;

      /**
       * @brief Configure the Random Sequence using the meta data present in the
       * AsciiHeader.
       *
       * @param header header containing a UTC_START and OBS_OFFSET key/val pair
       */
      void configure(const ska::pst::common::AsciiHeader& header);

      /**
       * @brief Reset the internal state of the Random Sequence.
       * The next call to generate or validate will behave as per the first call to these functions.
       *
       */
      void reset();

      /**
       * @brief Generate a random sequence of uniformly distributed unsigned 8-bit integers.
       * Each random number generated advances the sequence of random numbers.
       *
       * @param buffer pointer to memory to which the random sequence should be written
       * @param bufsz number of elements to write to the buffer
       */
      void generate(uint8_t * buffer, uint64_t bufsz);

      /**
       * @brief Generate a random data sequence written to the provided buffer in blocks
       *
       * @param buffer buffer to write the random sequence to
       * @param bufsz size of the buffer in bytes
       * @param block_offset offset from the start of the buffer for the first block
       * @param block_size size of each block of random data to write in bytes
       * @param block_stride separate between each block of random data in bytes
       */
      void generate_block(uint8_t * buffer, uint64_t bufsz, uint64_t block_offset, uint64_t block_size, uint64_t block_stride);

      /**
       * @brief Compare contents of buffer to expected random sequence
       *
       * @param buffer pointer to buffer containing samples to be validated
       * @param bufsz size of the buffer to be validated
       * @return true if all samples are valid
       * @return false if any samples are invalid
       */
      auto validate(uint8_t * buffer, uint64_t bufsz) -> bool;

      /**
       * @brief Validate the n data sequence written to the provided buffer in blocks
       *
       * @param buffer pointer to buffer containing samples to be validated
       * @param bufsz size of the buffer to be validated
       * @param block_offset offset from the start of the buffer for the first block
       * @param block_size size of each block of n data to write in bytes
       * @param block_stride separate between each block of n data in bytes
       * @return true if all samples are valid
       * @return false if any samples are invalid
       */
      auto validate_block(uint8_t * buffer, uint64_t bufsz, uint64_t block_offset, uint64_t block_size, uint64_t block_stride) -> bool;

      /**
       * @brief Seek forward through the random sequence.
       *
       * @param nelements the number of elements in the sequence to skip over
       */
      void seek(uint64_t nelements);

    private:

      /**
       * @brief Search the buffer for the expected random sequence
       *
       * @param buffer pointer to buffer containing samples to be searched
       * @param bufsz size of the buffer to be searched
       * @param seqlen number of consecutive samples from the expected random sequence to be matched
       * @return offset from start of buffer at which expected sequence match is found
       * @return -1 if no match is found
       */
      auto search_buffer_for_expected_sequence(uint8_t * buffer, uint64_t bufsz, uint64_t seqlen) -> int64_t;

      /**
       * @brief Search the random sequence for the buffer contents
       *
       * @param buffer pointer to buffer containing samples to be matched
       * @param bufsz size of the buffer to be matched
       * @param max_offset maximum number of samples from the expected random sequence to be searched
       * @return offset from current point in random sequence ath which a match is found
       * @return -1 if no match is found
       */
      auto search_expected_sequence_for_buffer(uint8_t * buffer, uint64_t bufsz, uint64_t max_offset) -> int64_t;

      //! the seed used to initialise the generator
      uint64_t seed_value{0};

      //! the current byte/element offset of the sequence being validated
      uint64_t byte_offset{0};

      //! random number engine based on Mersenne Twister algorithm
      std::mt19937 generator;
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_RandomSequence_h
