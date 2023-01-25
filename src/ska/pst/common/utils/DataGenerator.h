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
#include "ska/pst/common/utils/DataLayout.h"

namespace ska::pst::common {

  /**
   * @brief Abstract base class for data+weights+scales generation and validation
   *
   */
  class DataGenerator
  {
    public:

      /**
       * @brief Construct a new DataGenerator object
       *
       */
      DataGenerator() = default;

      /**
       * @brief Destroy the DataGenerator object
       *
       */
      virtual ~DataGenerator() = default;

      /**
       * @brief Configure the streams written to data+weights+scales
       *
       * @param config the keyword/value pairs used to configure the data+weights+scales streams
       */
      virtual void configure(const ska::pst::common::AsciiHeader& config);

      /**
       * @brief Configure the offsets and sizes of data+weights+scales in each packet
       *
       * @param layout DataLayout that defines the packet structure
       */
      void copy_layout(const ska::pst::common::DataLayout* layout);

      /**
       * @brief Fill the data+weights+scales of the next UDP packet
       *
       * @param buf base memory address of the packet to be filled
       */
      virtual void fill_block(char * buf);

      /**
       * @brief Fill the data stream in the provided buffer
       *
       * @param buffer pointer to buffer to be filled with sequence of data elements
       */
      virtual void fill_data(char * buf, uint64_t size) = 0;

      /**
       * @brief Fill the weights stream in the provided buffer
       *
       * @param buffer pointer to buffer to be filled with sequence of weight elements
       */
      virtual void fill_weights(char * buf, uint64_t size) = 0;

      /**
       * @brief Fill the scales stream in the provided buffer
       *
       * @param buffer pointer to buffer to be filled with sequence of scale elements
       */
      virtual void fill_scales(char * buf, uint64_t size) = 0;

      /**
       * @brief Verify the data+weights+scales of the received UDP packet
       *
       * @param buffer pointer to buffer containing received UDP packet
       * @return true if both data and weights match expectations
       */
      virtual bool test_block(char * buf);

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

      /**
       * @brief Verify the scales stream in the provided buffer
       *
       * @param buffer pointer to buffer containing sequence of scale elements
       * @return true if scales match expectations
       */
      virtual bool test_scales(char * buf, uint64_t size) = 0;

      /**
       * @brief Reset all sequences (data, weights, and scales)
       * The next call to fill_block or test_block will behave as per the first call to these functions.
       *
       */
      virtual void reset() = 0;

    protected:

      //! Layout of each block of data
      DataLayout layout;

      //! flag set when block/packet memory layout has been configured
      bool layout_configured{false};
  };

} // ska::pst::common

#endif // SKA_PST_COMMON_UTILS_DataGenerator_h

