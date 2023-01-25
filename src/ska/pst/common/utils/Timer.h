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

#include <sys/time.h>
#include <cinttypes>

#ifndef SKA_PST_COMMON_UTIL_Timer_h
#define SKA_PST_COMMON_UTIL_Timer_h

namespace ska::pst::common
{
  /**
   * @brief Provides functionality for millisecond precision timing
   * 
   */
  class Timer {

    public: 

      /**
       * @brief Construct a new Time object initialized to the current timestamp
       * 
       */
      Timer();

      /**
       * @brief Destroy the Timer object
       *
       */
      ~Timer() = default;

      /**
       * @brief Reset the timer.
       *
       */
      void reset();

      /**
       * @brief Wait until the specified offset has occured since the last reset of the timer
       *
       * @param offset offset in microseconds
       */
      void wait_until(double offset);

      /**
       * @brief Get the elapsed time since the last reset in microseconds
       *
       * @return double elapsed time in microseconds
       */
      double get_elapsed_microseconds();

      /**
       * @brief Get the elapsed time since the last reset in milliseconds
       *
       * @return int elapsed time in milliseconds
       */
      int get_elapsed_milliseconds();

      /**
       * @brief Print information about the data transfer performance
       *
       * @param bytes number of bytes to use when calculating effective data rate
       */
      void print_rates(uint64_t bytes);
    
    private:

      struct timeval start_epoch{};

      double target{0};

  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTIL_Timer_h
