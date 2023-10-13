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

#include "ska/pst/common/utils/PacketGenerator.h"
#include "ska/pst/common/utils/UniformSequence.h"

#ifndef SKA_PST_COMMON_UTILS_ScaleWeightGenerator_h
#define SKA_PST_COMMON_UTILS_ScaleWeightGenerator_h

namespace ska::pst::common {

  /**
   * @brief Generates and validates weights and scales using a UniformSequence (unity values)
   *
   */
  class ScaleWeightGenerator : public PacketGenerator
  {
    public:

      /**
       * @brief Construct a new ScaleWeightGenerator object
       *
       */
      explicit ScaleWeightGenerator(std::shared_ptr<PacketLayout> layout);

      /**
       * @brief Destroy the ScaleWeightGenerator object
       *
       */
      ~ScaleWeightGenerator() override = default;

      /**
       * @brief Configure the streams written to weights and scales
       *
       * @param config ignored - the weights and scales sequences are simply reset
       */
      void configure(const ska::pst::common::AsciiHeader& config) override;

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
       * @brief Reset weights and scales scales sequences
       * The next call to fill_weights|scales or test_weights|scales will behave as per the first call to these functions.
       *
       */
      void reset() override;

    private:
    
      //! sequence of uniform values for the weights
      UniformSequence<char> wts_sequence;

      //! sequence of randomly distributed values for the scales
      UniformSequence<float> scl_sequence;

  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_ScaleWeightGenerator_h

