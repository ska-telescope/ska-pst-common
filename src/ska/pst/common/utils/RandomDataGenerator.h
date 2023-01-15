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

#include "ska/pst/common/utils/DataGenerator.h"
#include "ska/pst/common/utils/RandomSequence.h"

#ifndef SKA_PST_COMMON_UTILS_RandomDataGenerator_h
#define SKA_PST_COMMON_UTILS_RandomDataGenerator_h

namespace ska::pst::common {

  /**
   * @brief Generates and validates data + weights using a RandomSequence for each
   * 
   */
  class RandomDataGenerator : public DataGenerator
  {
    public:

      /**
       * @brief Construct a new RandomDataGenerator object
       * 
       */
      RandomDataGenerator() = default;

      /**
       * @brief Destroy the RandomDataGenerator object
       * 
       */
      ~RandomDataGenerator() = default;

      /**
       * @brief Configure the streams written to data + weights
       *
       * @param config contains the UTC_START and OFFSET keyword/value pairs required to configure the data+weights streams
       */
      void configure(const ska::pst::common::AsciiHeader& config) override;

      /**
       * @brief Fill the data + weights of the next UDP packet
       *
       * @param buf base memory address of the packet to be filled
       */
      void fill_data_and_weights (char * buf) override;

      /**
       * @brief Verify the data stream in the provided buffer
       * 
       * @param buffer pointer to buffer containing sequence of received UDP packets
       * @return true if data match expectations
       */
      virtual bool test_data (char * buf, uint64_t size) override;

      /**
       * @brief Verify the weights stream in the provided buffer
       * 
       * @param buffer pointer to buffer containing sequence of received UDP packets
       * @return true if weights match expectations
       */
      virtual bool test_weights (char * buf, uint64_t size) override;

    private:

      ska::pst::common::RandomSequence dat_sequence;
      ska::pst::common::RandomSequence wts_sequence;

  };

} // ska::pst::common

#endif // SKA_PST_COMMON_UTILS_RandomDataGenerator_h

