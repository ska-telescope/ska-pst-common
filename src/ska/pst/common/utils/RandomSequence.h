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

#include <inttypes.h>
#include <random>

#include "ska/pst/common/utils/AsciiHeader.h"

#ifndef SKA_PST_COMMON_UTILS_RandomSequence_h
#define SKA_PST_COMMON_UTILS_RandomSequence_h

namespace ska::pst::common {

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
       * @brief Compare contents of buffer to expected random sequence
       * 
       * @param buffer pointer to buffer containing samples to be validated
       * @param bufsz size of the buffer to be validated
       * @return true if all samples are valid
       * @return false if any samples are invalid
       */
      bool validate(uint8_t * buffer, uint64_t bufsz);

      /**
       * @brief Seek forward through the random sequence.
       * 
       * @param nelements the number of elements in the sequence to skip over
       */
      void seek(uint64_t nelements);

      //! verbosity flag used during debugging
      bool verbose;
      
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
      int64_t search_buffer_for_expected_sequence(uint8_t * buffer, uint64_t bufsz, uint64_t seqlen);

      /**
       * @brief Search the random sequence for the buffer contents
       *
       * @param buffer pointer to buffer containing samples to be matched
       * @param bufsz size of the buffer to be matched
       * @param max_offset maximum number of samples from the expected random sequence to be searched
       * @return offset from current point in random sequence ath which a match is found
       * @return -1 if no match is found
       */
      int64_t search_expected_sequence_for_buffer(uint8_t * buffer, uint64_t bufsz, uint64_t max_offset);

      //! the seed used to initialise the generator
      uint64_t seed_value{0};

      //! the current byte/element offset of the sequence being validated
      uint64_t byte_offset{0};

      //! random number engine based on Mersenne Twister algorithm
      std::mt19937 generator;
  };

} // ska::pst::common

#endif // SKA_PST_COMMON_UTILS_RandomSequence_h
