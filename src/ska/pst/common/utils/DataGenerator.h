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

#ifndef SKA_PST_COMMON_UTILS_DataGenerator_h
#define SKA_PST_COMMON_UTILS_DataGenerator_h

#include "ska/pst/common/utils/AsciiHeader.h"

namespace ska::pst::common {

  /**
   * @brief Abstract base class for data + weights generation and validation
   * 
   */
  class DataGenerator
  {
    public:

      /**
       * @brief Construct a new DataGenerator object
       * 
       */
      DataGenerator();

      /**
       * @brief Destroy the DataGenerator object
       * 
       */
      virtual ~DataGenerator() = default;

      /**
       * @brief Configure the streams written to data + weights
       * 
       * @param config the keyword/value pairs used to configure the data+weights streams
       */
      virtual void configure(const ska::pst::common::AsciiHeader& config);

      /**
       * @brief Fill the data + weights of the next UDP packet
       * 
       * @param buf base memory address of the packet to be filled
       */
      virtual void fill_data_and_weights(char * buf) = 0;

      /**
       * @brief Verify the data + weights of the received UDP packet
       * 
       * @param buffer pointer to buffer containing received UDP packet
       * @return true if both data and weights match expectations
       */
      virtual bool test_data_and_weights(char * buf);

      /**
       * @brief Verify the data stream in the provided buffer
       * 
       * @param buffer pointer to buffer containing sequence of data elements
       * @return true if data match expectations
       */
      virtual bool test_data(char * buf, uint64_t size) = 0;

      /**
       * @brief Verify the weights stream in the provided buffer
       * 
       * @param buffer pointer to buffer containing sequence of weight elements
       * @return true if weights match expectations
       */
      virtual bool test_weights(char * buf, uint64_t size) = 0;

    protected:

      //! size of the header in the UDP packet payload in bytes
      unsigned packet_header_size{0};

      //! size of the data in the UDP packet payload in bytes
      unsigned packet_data_size{0};

      //! size of the weights in the UDP packet payload in bytes
      unsigned packet_weights_size{0};

      //! size of the scales in the UDP packet payload in bytes
      unsigned packet_scales_size{0};

      //! offset from first byte of UDP packet payload for the data
      unsigned packet_data_offset{0};

      //! offset from first byte of UDP packet payload for the weights
      unsigned packet_weights_offset{0};

      //! offset from first byte of UDP packet payload for the scales
      unsigned packet_scales_offset{0};

      //! flag for format configuration, fixed time parameters received
      bool configured{false};

  };

} // ska::pst::common

#endif // SKA_PST_COMMON_UTILS_DataGenerator_h

