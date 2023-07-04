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

#ifndef SKA_PST_COMMON_UTILS_DataLayout_h
#define SKA_PST_COMMON_UTILS_DataLayout_h

namespace ska::pst::common {

  /**
   * @brief Stores the offsets and sizes of data, weights, and scales in data blocks
   *
   */
  class DataLayout
  {
    public:

      /**
       * @brief Construct a new DataLayout object
       *
       */
      DataLayout() = default;

      /**
       * @brief Destroy the DataLayout object
       *
       */
      ~DataLayout() = default;

      /**
       * @brief Get the total size of the data block
       *
       * @return unsigned total size of data block
       */
      [[nodiscard]] auto get_packet_size() const -> unsigned { return packet_size; };

      /**
       * @brief Get the size of the header in each data block.
       *
       * @return unsigned size of header block in data block
       */
      [[nodiscard]] auto get_packet_header_size() const -> unsigned { return packet_header_size; };

      /**
       * @brief Get the size of the data in each data block.
       *
       * @return unsigned size of data samples in data block
       */
      [[nodiscard]] auto get_packet_data_size() const -> unsigned { return packet_data_size; };

      /**
       * @brief Get the size of the weights in each data block.
       *
       * @return unsigned size of the weights data in the data block
       */
      [[nodiscard]] auto get_packet_weights_size() const -> unsigned { return packet_weights_size; };

     /**
       * @brief Get the size of the scale factor in each data block.
       *
       * @return unsigned size of the scale factor in the data block
       */
      [[nodiscard]] auto get_packet_scales_size() const -> unsigned { return packet_scales_size; };

      /**
       * @brief Get the offset of the data in each data block.
       *
       * @return unsigned offset of data samples in data block
       */
      [[nodiscard]] auto get_packet_data_offset() const -> unsigned { return packet_data_offset; };

      /**
       * @brief Get the offset of the weights in each data block.
       *
       * @return unsigned offset of the weights data in the data block
       */
      [[nodiscard]] auto get_packet_weights_offset() const -> unsigned { return packet_weights_offset; };

      /**
       * @brief Get the offset of the scales in each data block.
       *
       * @return unsigned offset of scales in data block
       */
      [[nodiscard]] auto get_packet_scales_offset() const -> unsigned { return packet_scales_offset; };

      /**
       * @brief Get the number of samples per UDP packet
       *
       * @return unsigned number of samples in each UDP packet
       */
      [[nodiscard]] auto get_samples_per_packet() const -> unsigned { return nsamp_per_packet; };

      /**
       * @brief Get the number of channels per UDP packet
       *
       * @return unsigned number of channels in each UDP packet
       */
      [[nodiscard]] auto get_nchan_per_packet() const -> unsigned { return nchan_per_packet; };

      /**
       * @brief Get the number of samples per weight in each UDP packet
       *
       * @return unsigned number of samples per weight in each UDP packet
       */
      [[nodiscard]] auto get_nsamp_per_weight() const -> unsigned { return nsamp_per_weight; };

    protected:

      //! size of the data block
      unsigned packet_size{0};

      //! size of the header in each block in bytes
      unsigned packet_header_size{0};

      //! size of the data in each block in bytes
      unsigned packet_data_size{0};

      //! size of the weights in each block in bytes
      unsigned packet_weights_size{0};

      //! size of the scale factors in each block in bytes
      unsigned packet_scales_size{0};

      //! offset from first byte of each block for the data
      unsigned packet_data_offset{0};

      //! offset from first byte of each block for the weights
      unsigned packet_weights_offset{0};

      //! offset from first byte of each block for the scale factor
      unsigned packet_scales_offset{0};

      //! Number of samples per UDP packet
      unsigned nsamp_per_packet{0};

      //! Number of channels per UDP packet
      unsigned nchan_per_packet{0};

      //! Number of samples per relative weight
      unsigned nsamp_per_weight{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_DataLayout_h

