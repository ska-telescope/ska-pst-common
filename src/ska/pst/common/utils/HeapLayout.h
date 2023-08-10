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

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/utils/PacketLayout.h"
#include "ska/pst/common/definitions.h"

#include <memory>

#ifndef SKA_PST_COMMON_HeapLayout_h
#define SKA_PST_COMMON_HeapLayout_h

namespace ska::pst::common
{

  /**
   * @brief Stores the offsets and sizes of data, weights, and scales in heaps of packets
   *
   * Data and weights are stored in separate heaps in blocks of shared memory or data from file
   */
  class HeapLayout
  {
    public:

      /**
       * @brief Construct a new HeapLayout object
       *
       */
      HeapLayout() = default;

      /**
       * @brief Destroy the HeapLayout object
       *
       */
      ~HeapLayout() = default;

      /**
       * @brief Configure a HeapLayout from the AsciiHeader from the data and weights streams
       *
       * @param data_config AsciiHeader containing the configuration of the data stream
       * @param weights_config AsciiHeader containing the configuration of the weights stream
       * 
       */
      void configure (const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config);

      /**
       * @brief Initialise a HeapLayout from the AsciiHeader from the data and weights streams
       *
       * @param data_config AsciiHeader containing the configuration of the data stream
       * @param weights_config AsciiHeader containing the configuration of the weights stream
       * 
       * The RESOLUTION parameters of both data_config and weights_config are initialised
       */
      void initialise (ska::pst::common::AsciiHeader& data_config, ska::pst::common::AsciiHeader& weights_config);

      /**
       * @brief Get the layout of each packet in the heap
       *
       * @return PacketLayout layout of each packet in the heap
       */
      auto get_packet_layout() const -> const PacketLayout& { return *packet_layout; }

      /**
       * @brief Get the pointer to the layout of each packet in the heap
       *
       * @return PacketLayout pointer to the layout of each packet in the heap
       */
      auto get_packet_layout_ptr() const -> const std::shared_ptr<PacketLayout>& { return packet_layout; }

      /**
       * @brief Get the number of packets in each heap
       *
       * @return unsigned number of packets in each heap
       */
      auto get_packets_per_heap() const -> unsigned { return packets_per_heap; }

      /**
       * @brief Get the number of bytes in each packet of data stream
       *
       * @return unsigned number of bytes in each packet of data stream
       */
      auto get_data_packet_stride() const -> unsigned { return data_packet_stride; }

      /**
       * @brief Get the number of bytes in each heap of data stream
       *
       * @return unsigned number of bytes in heap packet of data stream
       */
      auto get_data_heap_stride() const -> unsigned { return data_heap_stride; }

      /**
       * @brief Get the number of bytes in each packet of weights stream
       *
       * @return unsigned number of bytes in each packet of weights stream
       */
      auto get_weights_packet_stride() const -> unsigned { return weights_packet_stride; }

      /**
       * @brief Get the number of bytes in each heap of weights stream
       *
       * @return unsigned number of bytes in heap packet of weights stream
       */
      auto get_weights_heap_stride() const -> unsigned { return weights_heap_stride; }

    protected:

      //! The layout of each packet in the heap
      std::shared_ptr<PacketLayout> packet_layout;

      //! Number of packets per heap in the data stream
      uint32_t packets_per_heap{0};

      //! Number of bytes per packet in the data stream
      uint32_t data_packet_stride{0};

      //! Number of bytes per packet in the weights stream
      uint32_t weights_packet_stride{0};

      //! Number of bytes per heap in the data stream
      uint32_t data_heap_stride{0};

      //! Number of bytes per heap in the weights stream
      uint32_t weights_heap_stride{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_HeapLayout_h
