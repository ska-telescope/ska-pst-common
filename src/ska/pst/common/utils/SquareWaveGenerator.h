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

#include "ska/pst/common/utils/GaussianNoiseGenerator.h"

#ifndef SKA_PST_COMMON_UTILS_SquareWaveGenerator_h
#define SKA_PST_COMMON_UTILS_SquareWaveGenerator_h

namespace ska::pst::common {

  /**
   * @brief Generates and validates normally-distributed noise that is amplitude-modulated using a square wave
   * with configurable modulation period, duty cycle, and on-pulse amplitude that may vary linearly
   * as a function of frequency channel independently in each polarisation
   *
   */
  class SquareWaveGenerator : public GaussianNoiseGenerator
  {
    public:

      /**
       * @brief Construct a new SquareWaveGenerator object
       *
       */
      explicit SquareWaveGenerator(std::shared_ptr<PacketLayout> layout);

      /**
       * @brief Destroy the SquareWaveGenerator object
       *
       */
      ~SquareWaveGenerator() override = default;

      /**
       * @brief Configure the streams written to data + weights
       *
       * @param config contains the following parameters
       * 
       * - UTC_START used to seed the random number generator (mandatory)
       * - CAL_DUTY_CYCLE the fraction of the square wave period in the on-pulse state (optional; must be greater than 0 and less than 1)
       * - CALFREQ the frequency (inverse of the period) of the square wave (optional; must be greater than zero)
       * 
       * The following intensity configuration parameters are all optional and must be greater than zero:
       * 
       * - CAL_OFF_INTENSITY off-pulse intensity for all polarizations and frequency channels
       * - CAL_ON_INTENSITY on-pulse intensity for all polarizations and frequency channels
       * - CAL_ON_POL_0_INTENSITY on-pulse intensity for polarization 0 and all frequency channels
       * - CAL_ON_POL_1_INTENSITY on-pulse intensity for polarization 1 and all frequency channels
       * - CAL_ON_CHAN_0_INTENSITY on-pulse intensity for all polarizations at frequency channel zero
       * - CAL_ON_CHAN_N_INTENSITY on-pulse intensity for all polarizations at the number of frequency channels
       * - CAL_ON_POL_0_CHAN_0_INTENSITY on-pulse intensity for polarization 0 at frequency channel zero
       * - CAL_ON_POL_0_CHAN_N_INTENSITY on-pulse intensity for polarization 0 at the number of frequency channels
       * - CAL_ON_POL_1_CHAN_0_INTENSITY on-pulse intensity for polarization 1 at frequency channel zero
       * - CAL_ON_POL_1_CHAN_N_INTENSITY on-pulse intensity for polarization 1 at the number of frequency channels
       * 
       * If any one of the above CHAN_0 intensities is specified, then the matching CHAN_N intensity must also be specified.
       * Each CHAN_0,CHAN_N pair defines an intensity gradient that will be applied to all frequency channels.
       * If any intensity (in any polarization or frequency channel) is multiply defined, then the intensity 
       * configuration parameters included later in the above list will override any configuration set by parameters
       * included earlier in the list.
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
       * @brief Verify the data stream in the provided buffer
       *
       * @param buf pointer to buffer containing sequence of data to be verified
       * @param size number of bytes in buffer to be tested
       *
       * @return true if data match expectations
       */
      auto test_data(char * buf, uint64_t size) -> bool override;

      /**
       * @brief Set the on-pulse intensity for all polarizations and frequency channels
       *
       * @param intensity the intensity for all polarizations and frequency channels
       *
       */
      void set_on_intensity(float intensity);

      /**
       * @brief Set the on-pulse intensity for all polarizations with a slope in frequency channel
       *
       * @param intensity0 the intensity for all polarizations at frequency channel zero
       * @param intensityN the intensity for all polarizations at frequency channel N-1
       *
       */
      void set_on_intensity(float intensity0, float intensityN);

      /**
       * @brief Set the on-pulse intensity for the specified polarization and all frequency channels
       *
       * @param intensity the intensity for the specified polarization and all frequency channels
       *
       */
      void set_on_intensity_pol(unsigned ipol, float intensity);

      /**
       * @brief Set the on-pulse intensity for the specified polarization with a slope in frequency channel
       *
       * @param intensity0 the intensity for the specified polarization at frequency channel zero
       * @param intensityN the intensity for the specified polarization at frequency channel N-1
       *
       */
      void set_on_intensity_pol(unsigned ipol, float intensity0, float intensityN);

    private:

      //! Frequency of square wave (inverse of period) in Hz
      double frequency{1.0};

      //! Sampling interval in seconds
      double sampling_interval{0.0};

      //! Fraction of period in the on-pulse state
      double duty_cycle{0.5};

      //! Standard deviation of off-pulse noise
      float off_stddev{10.0};

      //! Standard deviation of on-pulse noise
      float default_on_stddev{11.0};

      //! Standard deviations of on-pulse noise as a function of polarization and frequency
      std::vector<std::vector<float>> on_stddev;

      //! Resize the on_stddev array
      void resize_on_stddev(float set_stddev = 0.0);

      //! Set all values in the on_stddev array to a single value
      void set_on_stddev(float stddev);

      //! Set all values in the on_stddev array for the specified polarization to a single value
      void set_on_stddev_pol(unsigned ipol, float stddev);

      //! Convert intensity to standard deviation
      float intensity_to_stddev(float intensity);

      //! Current sample in the sequence
      uint64_t current_sample{0};   

      //! Current channel in the heap
      uint32_t current_channel{0};

      //! Temporary buffer used by test_data
      std::vector<char> temp_data;
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_SquareWaveGenerator_h

