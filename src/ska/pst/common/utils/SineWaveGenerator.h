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
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/DataGenerator.h"

#ifndef SKA_PST_COMMON_UTILS_SineWaveGenerator_h
#define SKA_PST_COMMON_UTILS_SineWaveGenerator_h

namespace ska::pst::common {

  /**
   * @brief Generates and validates data + weights using a sine wave for each
   *
   */
  class SineWaveGenerator : public DataGenerator
  {
    public:

      /**
       * @brief Construct a new SineWaveGenerator object
       *
       */
      SineWaveGenerator(std::shared_ptr<DataLayout> layout);

      /**
       * @brief Destroy the SineWaveGenerator object
       *
       */
      ~SineWaveGenerator() = default;

      /**
       * @brief Configure the streams written to data + weights
       *
       * @param config contains the UTC_START and OFFSET keyword/value pairs required to configure the data+weights streams
       */
      void configure(const ska::pst::common::AsciiHeader& config) override;

      /**
       * @brief Fill the buffer with a sequence of data
       *
       * @param buf base memory address of the buffer to be filled
       * @param size number of bytes to be written to buffer
       */
      void fill_data(char * buf, uint64_t size) override;

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
       * @brief Verify the data stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of data to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if data match expectations
       */
      virtual bool test_data(char * buf, uint64_t size) override;

      /**
       * @brief Verify the weights stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of weights to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if weights match expectations
       */
      virtual bool test_weights(char * buf, uint64_t size) override;

      /**
       * @brief Verify the scales stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of scale factors to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if scales match expectations
       */
      virtual bool test_scales(char * buf, uint64_t size) override;

      /**
       * @brief Reset all sequences (data, weights, and scales)
       * The next call to fill_block or test_block will behave as per the first call to these functions.
       *
       */
      virtual void reset() override;

    private:

      /* Current offset from t=0, updated on each call to fill_data or test_data */
      uint64_t current_sample{0};

      /* Period of sine wave in samples (irrational number ~100, finishes ~10 cycles in 1k points) */
      double period{M_PI * M_PI * M_PI * M_PI};

      template <typename T>
      std::complex<T> next_sample(bool generate)
      {
        if (!generate)
        {
          return std::complex<T>(0, 0);
        }
        double phase = current_sample / period;
        current_sample++;
        return std::complex<T>(T(amplitude * sinf(phase)), T(amplitude * cosf(phase)));
      }

      template <typename T>
      void fill_complex_data(char * buf, uint64_t size)
      {
        std::complex<T> * ptr = reinterpret_cast<std::complex<T> *>(buf);
        static constexpr uint32_t nbits_per_byte = 8;
        const uint32_t nsamp_per_packet = layout->get_samples_per_packet();
        const uint32_t nchan_per_packet = layout->get_nchan_per_packet();
        const uint32_t resolution = nsamp_per_packet * nchan_per_packet * npol * ndim * nbit / nbits_per_byte;
        const uint32_t nblocks = size / resolution;
        SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::fill_complex_data nsamp_per_packet={} nchan_per_packet={} size={} resolution={} nblocks={}", nsamp_per_packet, nchan_per_packet, size, resolution, nblocks);

        uint64_t i = 0;
        for (uint32_t iblock=0; iblock<nblocks; iblock++)
        {
          for (uint32_t ipol=0; ipol<npol; ipol++)
          {
            for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
            {
              const uint32_t ochan = current_channel + ichan;
              const bool active_chan = (ochan == sinusoid_channel);
              for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp++)
              {
                ptr[i] = next_sample<T>(active_chan);
                i++;
              }
            }
          }
          current_channel += nchan_per_packet;
          if (current_channel >= nchan)
          {
            current_channel = 0;
          }
        }
      }

      template <typename T>
      bool test_complex_data(char * buf, uint64_t size)
      {
        std::complex<T> * ptr = reinterpret_cast<std::complex<T> *>(buf);
        static constexpr uint32_t nbits_per_byte = 8;
        const uint32_t nsamp_per_packet = layout->get_samples_per_packet();
        const uint32_t nchan_per_packet = layout->get_nchan_per_packet();
        const uint32_t resolution = nsamp_per_packet * nchan_per_packet * npol * ndim * nbit / nbits_per_byte;
        const uint32_t nblocks = size / resolution;

        SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::test_complex_data nsamp_per_packet={} nchan_per_packet={} size={} resolution={} nblocks={}", nsamp_per_packet, nchan_per_packet, size, resolution, nblocks);

        uint64_t i = 0;
        for (uint32_t iblock=0; iblock<nblocks; iblock++)
        {
          for (uint32_t ipol=0; ipol<npol; ipol++)
          {
            for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
            {
              const uint32_t ochan = current_channel + ichan;
              const bool active_chan = (ochan == sinusoid_channel);
              for (uint32_t isamp=0; isamp<nsamp_per_packet; isamp++)
              {
                std::complex<T> val = next_sample<T>(active_chan);
                if (ptr[i] != val)
                {
                  return false;
                }
                i++;
              }
            }
          }
          current_channel += nchan_per_packet;
          if (current_channel >= nchan)
          {
            current_channel = 0;
          }
        }
        return true;
      }

      uint32_t sinusoid_channel{0};

      double amplitude{0};

      uint32_t current_channel{0};
  };

} // ska::pst::common

#endif // SKA_PST_COMMON_UTILS_SineWaveGenerator_h

