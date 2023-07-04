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

#include <complex>

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/utils/DataLayout.h"

#ifndef SKA_PST_COMMON_UTILS_DataUnpacker_h
#define SKA_PST_COMMON_UTILS_DataUnpacker_h

namespace ska::pst::common
{
  /**
   * @brief Unpacks data+weights+scales generation and validation
   *
   */
  class DataUnpacker
  {
    public:

      /**
       * @brief Construct a new DataUnpacker object
       *
       */
      DataUnpacker() = default;

      /**
       * @brief Destroy the DataUnpacker object
       *
       */
      virtual ~DataUnpacker() = default;

      /**
       * @brief Configure the data unpacker with the AsciiHeader from the data and weights streams
       *
       * @param data_config AsciiHeader containing the configuration of the data stream
       * @param weights_config AsciiHeader containing the configuration of the weights stream
       */
      virtual void configure(const ska::pst::common::AsciiHeader& data_config, const ska::pst::common::AsciiHeader& weights_config);

      /**
       * @brief Unpack the data and weights streams into a floating point vector
       *
       * @param data pointer to raw data array
       * @param data_bufsz size of the raw data array in bytes
       * @param weights pointer to raw weights array
       * @param weights_bufsz size of the raw weights array in bytes
       * @return std::vector<std::vector<std::vector<std::complex<float>>>>& unpacked data ordered by time, freqeuncy then polarisation
       */
      auto unpack(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz) -> std::vector<std::vector<std::vector<std::complex<float>>>> &;

      /**
       * @brief Integrate the data and weights streams into an internal floating point bandpass vector
       *
       * @param data pointer to raw data array
       * @param data_bufsz size of the raw data array in bytes
       * @param weights pointer to raw weights array
       * @param weights_bufsz size of the raw weights array in bytes
       */
      void integrate_bandpass(char * data, uint64_t data_bufsz, char *weights, uint64_t weights_bufsz);

      /**
       * @brief Get the integrated bandpass vector
       *
       * @return std::vector<std::vector<float>>&o upacked bandpass vector ordered by frequency then polarisation
       */
      auto get_bandpass() -> std::vector<std::vector<float>>& { return bandpass; };

      /**
       * @brief Reset the integrated bandpass vector to zero
       *
       */
      void reset();

    protected:

    private:

      template <typename T>
      void unpack_samples(const T* in, char * weights, uint32_t nheaps)
      {
        // Unpack quantised data store in heap, packet, pol, chan_block, samp_block ordering used in CBF/PSR formats
        uint32_t packet_number = 0;
        for (uint32_t iheap=0; iheap<nheaps; iheap++)
        {
          for (uint32_t ipacket=0; ipacket<packets_per_heap; ipacket++)
          {
            const float scale_factor = get_scale_factor(weights, packet_number);
            if (std::isnan(scale_factor))
            {
              invalid_packets++;
            }
            for (uint32_t ipol=0; ipol<npol; ipol++)
            {
              for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
              {
                const uint32_t ochan = (ipacket * nchan_per_packet) + ichan;
                for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp++)
                {
                  if (std::isnan(scale_factor))
                  {
                    invalid_samples++;
                  }
                  else
                  {
                    uint32_t osamp = (iheap * nsamp_per_packet) + isamp;
                    unpacked[osamp][ochan][ipol] = std::complex<float>(static_cast<float>(in[0]), static_cast<float>(in[1])) / scale_factor; // NOLINT
                  }
                  in += 2; // NOLINT
                }
              }
            }
            packet_number++;
          }
        }
      }

      template <typename T>
      void integrate_samples(const T* in, char * weights, uint32_t nheaps)
      {
        // Unpack quantised data store in heap, packet, pol, chan_block, samp_block ordering
        // used in CBF/PSR formats
        uint32_t packet_number = 0;
        std::complex<float> sample{0,0};
        for (uint32_t iheap=0; iheap<nheaps; iheap++)
        {
          for (uint32_t ipacket=0; ipacket<packets_per_heap; ipacket++)
          {
            const float scale_factor = get_scale_factor(weights, packet_number);
            if (std::isnan(scale_factor))
            {
              invalid_packets++;
            }

            for (uint32_t ipol=0; ipol<npol; ipol++)
            {
              for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
              {
                const uint32_t ochan = (ipacket * nchan_per_packet) + ichan;
                for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp++)
                {
                  if (std::isnan(scale_factor))
                  {
                    invalid_samples++;
                  }
                  else
                  {
                    sample = std::complex<float>(static_cast<float>(in[0]), static_cast<float>(in[1])) / scale_factor;  // NOLINT
                    // integrate the power in each complex sample into the bandpass
                    float power = sample.real() * sample.real() + sample.imag() * sample.imag();
                    bandpass[ochan][ipol] += power;
                  }
                  in += 2; // NOLINT
                }
              }
            }
            packet_number++;
          }
        }
      }

      //! Unpacked data vector
      std::vector<std::vector<std::vector<std::complex<float>>>> unpacked;

      //! Integrated bandpass
      std::vector<std::vector<float>> bandpass;

      //! Resize the internal storage of the unpacked and bandpass vectors
      void resize(uint64_t data_bufsz);

      //! Return the scale factor packed into the weights array, corresponding to the provided packet number
      auto get_scale_factor(char * weights, uint32_t packet_number) -> float;

      //! Number of polarisations in the data stream
      uint32_t npol{0};

      //! Number of dimensions in the data stream
      uint32_t ndim{0};

      //! Number of channels in the data stream
      uint32_t nchan{0};

      //! Number of bits per sample in the data stream
      uint32_t nbit{0};

      //! Number of bits per sample in the weights stream
      uint32_t weights_nbit{0};

      //! Number of samples per UDP packet in the data stream
      uint32_t nsamp_per_packet{0};

      //! Number of channels per UDP packet in the data stream
      uint32_t nchan_per_packet{0};

      //! Number of samples per relative weight in the weights stream
      uint32_t nsamp_per_weight{0};

      //! Number of bytes per packet in the weights stream
      uint32_t weights_packet_stride{0};

      //! Size of a complete heap of data in the data stream, in btyes
      uint32_t heap_resolution{0};

      //! Size of the complex packet od adat ain the data stream, in bytes
      uint32_t packet_resolution{0};

      //! Number of UDP packets per heap in the data stream
      uint32_t packets_per_heap{0};

      //! Number of dropped packets (scale factor = NaN) encountered
      uint64_t invalid_packets{0};

      //! Number of invalid samples, arising from invalid packets
      uint64_t invalid_samples{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_DataUnpacker_h

