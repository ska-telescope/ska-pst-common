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

#include "ska/pst/common/utils/DataGenerator.h"
#include "ska/pst/common/utils/NormalSequence.h"
#include "ska/pst/common/utils/UniformSequence.h"
#include "ska/pst/common/utils/RandomSequence.h"

#ifndef SKA_PST_COMMON_UTILS_GaussianNoiseGenerator_h
#define SKA_PST_COMMON_UTILS_GaussianNoiseGenerator_h

namespace ska::pst::common {

  /**
   * @brief Generates and validates data + weights using a RandomSequence for each
   *
   */
  class GaussianNoiseGenerator : public DataGenerator
  {
    public:

      /**
       * @brief Construct a new GaussianNoiseGenerator object
       *
       */
      explicit GaussianNoiseGenerator(std::shared_ptr<DataLayout> layout);

      /**
       * @brief Destroy the GaussianNoiseGenerator object
       *
       */
      ~GaussianNoiseGenerator() override = default;

      /**
       * @brief Configure the streams written to data + weights
       *
       * @param config contains the UTC_START and OFFSET keyword/value pairs required to configure the data+weights streams
       */
      void configure(const ska::pst::common::AsciiHeader& config) override;

      /**
       * @brief Fill the buffer with a sequence of data
       *
       * @param buf base memory address of the buffer to be filled
       * @param size number of bytes to be written to buffer
       */
      void fill_data(char * buf, uint64_t size) override;

      /**
       * @brief Fill the buffer with a sequence of weights
       *
       * @param buf base memory address of the buffer to be filled
       * @param size number of bytes to be written to buffer
       */
      void fill_weights(char * buf, uint64_t size) override;

      /**
       * @brief Fill the buffer with a sequence of scale factors
       *
       * @param buf base memory address of the buffer to be filled
       * @param size number of bytes to be written to buffer
       */
      void fill_scales(char * buf, uint64_t size) override;

      /**
       * @brief Verify the data stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of data to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if data match expectations
       */
      auto test_data(char * buf, uint64_t size) -> bool override;

      /**
       * @brief Verify the weights stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of weights to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if weights match expectations
       */
      auto test_weights(char * buf, uint64_t size) -> bool override;

      /**
       * @brief Verify the scales stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of scale factors to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if scales match expectations
       */
      auto test_scales(char * buf, uint64_t size) -> bool override;

      /**
       * @brief Reset all sequences (data, weights, and scales)
       * The next call to fill_block or test_block will behave as per the first call to these functions.
       *
       */
      void reset() override;

    private:

      //! sequence of normally distributed values for the data samples
      ska::pst::common::NormalSequence dat_sequence;

      //! sequence of uniform values for the weights
      ska::pst::common::UniformSequence<char> wts_sequence;

      //! sequence of randomly distributed values for the scales
      ska::pst::common::RandomSequence scl_sequence;

      //! offset of the first scale heap in a weights+scales block in bytes
      uint64_t scl_block_offset{0};

      //! size of the scale heap in a weights+scales block in bytes
      uint64_t scl_block_size{0};

      //! offset of the first weight heap in a weights+scales block in bytes
      uint64_t wts_block_offset{0};

      //! size of a weight heap in a weights+scales block in bytes
      uint64_t wts_block_size{0};

      //! size of a scale and weights heap in bytes
      uint64_t block_stride{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_GaussianNoiseGenerator_h

