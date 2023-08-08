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

#include "ska/pst/common/utils/SegmentProducer.h"
#include "ska/pst/common/utils/PacketGenerator.h"
#include "ska/pst/common/utils/HeapLayout.h"

#include <spdlog/spdlog.h>
#include <complex>

#ifndef SKA_PST_COMMON_UTILS_SegmentGenerator_h
#define SKA_PST_COMMON_UTILS_SegmentGenerator_h

namespace ska::pst::common
{
  /**
   * @brief Generates simulated signals as a series of Segments
   *
   */
  class SegmentGenerator : public SegmentProducer
  {
    public:

      /**
       * @brief Construct a new SegmentGenerator object
       *
       */
      SegmentGenerator() = default;

      /**
       * @brief Destroy the SegmentGenerator object
       *
       */
      ~SegmentGenerator();

      /**
       * @brief Configure the simulator with the AsciiHeader for the data and weights+scales blocks
       *
       * @param data_config AsciiHeader containing the configuration of the data blocks
       * @param weights_config AsciiHeader containing the configuration of the weights blocks
       */
      virtual void configure(const AsciiHeader& data_config, const AsciiHeader& weights_config);

      /**
       * @brief Get the AsciiHeader that describes the data blocks
       *
       * @return const AsciiHeader& header of the data blocks
       */
      const AsciiHeader& get_data_header() const { return data_config; }

      /**
       * @brief Get the AsciiHeader that describes the weights blocks
       *
       * @return const AsciiHeader& header of the weights blocks
       */
      const AsciiHeader& get_weights_header() const { return weights_config; }

      /**
       * @brief Resize the internal storage for data and weights+scales
       *
       * @param nheap the number of heaps to be generated on each call to next_segment
       */
      void resize(uint64_t nheap);

      /**
       * @brief Get the next segment of simulated data and weights+scales
       *
       */
      Segment next_segment();

      /**
       * @brief Verify the data and weights+scales of the segment
       *
       * @param segment the segment of simulated data and weights+scales to be verified
       * @return true if data and weights+scales match expectations
       */
      bool test_segment(const Segment& segment);

      /**
       * @brief Reset all sequences (data, weights, and scales)
       * The next call to next_segment or test_segment will behave as on the first call to these functions
       *
       */
      void reset();

    protected:

      //! the AsciiHeader that describes the data blocks
      AsciiHeader data_config;
      
      //! the AsciiHeader that describes the weights blocks
      AsciiHeader weights_config;

      //! PacketGenerator sets the data and weights+scales of each packet in each heap
      std::shared_ptr<PacketGenerator> generator{nullptr};

      //! storage for the simulated Segment returned by next_segmemtn
      Segment segment;

      //! The layout of data, weights and scales in each heap
      HeapLayout layout;

      //! The number of heaps generated on each call to next_block
      uint64_t nheap{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_SegmentGenerator_h

