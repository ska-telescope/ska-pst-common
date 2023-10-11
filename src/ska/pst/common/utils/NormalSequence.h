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

#include <cinttypes>
#include <random>

#include "ska/pst/common/utils/AsciiHeader.h"
#include "ska/pst/common/definitions.h"

#ifndef SKA_PST_COMMON_UTILS_NormalSequence_h
#define SKA_PST_COMMON_UTILS_NormalSequence_h

namespace ska::pst::common {

  /**
   * @brief Generates a sequence of normally distributed integer values.
   * The configuration of the distribution is controlled by header supplied in the
   * configure method, through the following parameters:
   *   DISTRIBUTION_MEAN: mean of the values
   *   DISTRIBUTION_STDDEV: standard deviation of the values
   *   NBIT: Number of bits per sample in the values
   *
   */
  class NormalSequence {

    public:

      /**
       * @brief Construct a new Normal Sequence object
       *
       */
      NormalSequence() = default;

      /**
       * @brief Destroy the Normal Sequence object
       *
       */
      ~NormalSequence() = default;

      /**
       * @brief Configure the Normal Sequence using the meta data present in the
       * AsciiHeader.
       *
       * @param header header containing a UTC_START and OBS_OFFSET key/val pair
       */
      void configure(const ska::pst::common::AsciiHeader& header);

      /**
       * @brief Reset the internal state of the Normal Sequence.
       * The next call to generate or validate will behave as per the first call to these functions.
       *
       */
      void reset();

      /**
       * @brief Generate a random sequence of normally distributed unsigned 8-bit integers.
       * Each random number generated advances the sequence of random numbers.
       *
       * @param buffer pointer to memory to which the random sequence should be written
       * @param bufsz number of elements to write to the buffer
       */
      void generate(char * buffer, uint64_t bufsz);

      /**
       * @brief Compare contents of buffer to expected random sequence
       *
       * @param buffer pointer to buffer containing samples to be validated
       * @param bufsz size of the buffer to be validated
       * @return true if all samples are valid
       * @return false if any samples are invalid
       */
      auto validate(char * buffer, uint64_t bufsz) -> bool;

    private:

      //! default stddev
      static constexpr float default_stddev = 10;

      //! fraction of the previous red noise factor to use when computing the factor to apply to the next sample
      static constexpr float red_noise_percent_prev = 0.999;

      //! fraction of the current red noise factor to use when computing the factor to apply to the next sample
      static constexpr float red_noise_percent_new = 0.001;

      template <typename T>
      void generate_samples(T * out, uint64_t nval)
      {
        std::normal_distribution<float> distribution(mean, stddev);
        for (uint64_t i=0; i<nval; i++)
        {
          out[i] = static_cast<T>(get_val(distribution)); // NOLINT
        }
      }

      //! mean of the normal distribution
      float mean{0};

      //! standard deviation of the normal distribution
      float stddev{default_stddev};

      //! standard deviation of a red noise process
      float red_stddev{0};

      //! current red noise factor to be applied
      float red_noise_factor{1};

      //! new red noise factor to apply when generating the timeseries
      float new_red_noise_factor{0};

      //! get a 16-bit integer value from the normal distribution that is limited to min_val and max_val
      inline auto get_val(std::normal_distribution<float>& distribution) -> int16_t;

      //! number of bits per sample
      uint32_t nbit{0};

      //! minimum quantised value
      float min_val{0};

      //! maximum quantised value
      float max_val{0};

      //! the seed used to initialise the generator
      uint64_t seed_value{0};

      //! the current byte/element offset of the sequence being validated
      uint64_t byte_offset{0};

      //! random number engine based on Mersenne Twister algorithm
      std::mt19937 generator;

      //! random number engine based on Mersenne Twister algorithm
      std::mt19937 red_noise_generator;
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_NormalSequence_h
