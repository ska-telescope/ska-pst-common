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

#include "ska/pst/common/utils/BlockProducer.h"

#ifndef __SKA_PST_COMMON_UTILS_SegmentProducer_h
#define __SKA_PST_COMMON_UTILS_SegmentProducer_h

namespace ska::pst::common {

  /**
   * @brief Interface for reading blocks of voltage data and weights
   *
   * This pure virtual base class implements an interface to data+weights that can be from any source,
   * including file (see FileSegmentProducer), artificial signal generator (see SegmentGenerator) or ring buffer (in principal)
   */
  class SegmentProducer
  {
    public:

      /**
       * @brief Container of data and weights blocks
       *
       */
      class Segment
      {
        public:
          //! Data block
          BlockProducer::Block data;

          //! Weights block
          BlockProducer::Block weights;

          //! The offset, in bytes, of the data block, from the start of the stream
          size_t get_obs_offset() const { return data.obs_offset; }

      };

      /**
       * @brief Pure virtual destructor
       *
       */
      virtual ~SegmentProducer() = default;

      /**
       * @brief Get the AsciiHeader that describes the data block stream
       *
       * @return const ska::pst::common::AsciiHeader& header of the data block stream
       */
      virtual const ska::pst::common::AsciiHeader& get_data_header() const = 0;

      /**
       * @brief Get the AsciiHeader that describes the weights block stream
       *
       * @return const ska::pst::common::AsciiHeader& header of the weights block stream
       */
      virtual const ska::pst::common::AsciiHeader& get_weights_header() const = 0;

      /**
       * @brief Get the next segment of data and weights.
       *
       * The returned Segment contains the next blocks of data and weights.
       * Each Block includes the base pointer of an array and the size, in bytes, of that array.
       * End of data is indicated by a Block in which the pointers are set to nullptr and the sizes are set to zero.
       */
      virtual Segment next_segment() = 0;

  };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_UTILS_SegmentProducer_h
