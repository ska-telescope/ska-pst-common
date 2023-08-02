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

#include "ska/pst/common/utils/DataBlockSource.h"
#include "ska/pst/common/utils/HeapLayout.h"

#include <spdlog/spdlog.h>
#include <complex>

#ifndef SKA_PST_COMMON_UTILS_DataBlockGenerator_h
#define SKA_PST_COMMON_UTILS_DataBlockGenerator_h

namespace ska::pst::common
{
  /**
   * @brief Unpacks data+weights+scales generation and validation
   *
   */
  class DataBlockGenerator : public DataBlockSource
  {
    public:

      /**
       * @brief Construct a new DataBlockGenerator object
       *
       */
      DataBlockGenerator() = default;

      /**
       * @brief Destroy the DataBlockGenerator object
       *
       */
      ~DataBlockGenerator();

      /**
       * @brief Configure the data unpacker with the AsciiHeader from the data and weights streams
       *
       * @param data_config AsciiHeader containing the configuration of the data stream
       * @param weights_config AsciiHeader containing the configuration of the weights stream
       */
      virtual void configure(const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config);

      /**
       * @brief Get the AsciiHeader that describes the data block stream
       *
       * @return const ska::pst::common::AsciiHeader& header of the data block stream
       */
      const ska::pst::common::AsciiHeader& get_data_header() const { return data_config; }

      /**
       * @brief Get the AsciiHeader that describes the weights block stream
       *
       * @return const ska::pst::common::AsciiHeader& header of the weights block stream
       */
      const ska::pst::common::AsciiHeader& get_weights_header() const { return weights_config; }

      /**
       * @brief Resize the internal storage for data and weights
       *
       * @param nheap the number of heaps to be generated on each call to next_block
       */
      void resize(uint64_t nheap);

      /**
       * @brief Get the next block of data and weights.
       *
       * The returned Block contains the pointer to the next block of data
       * and weights.  It also includes the size, in bytes, for both data
       * and weights. Clients of this must not go beyond the size of data.
       */
      Block next_block();

    protected:

      //! the AsciiHeader that describes the data block stream
      ska::pst::common::AsciiHeader data_config;
      
      //! the AsciiHeader that describes the weights block stream
      ska::pst::common::AsciiHeader weights_config;

      //! the data and weights blocks
      Block block;

      //! The layout of data, weights and scales in each heap
      HeapLayout layout;

      //! The number of heaps generated on each call to next_block
      uint64_t nheap{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_DataBlockGenerator_h

