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

#include <cstddef>
#include <ctime>
#include <string>

#include "ska/pst/common/definitions.h"

#ifndef SKA_PST_COMMON_UTILS_Time_h
#define SKA_PST_COMMON_UTILS_Time_h

#define MJD_1970_01_01 40587  // NOLINT
#define UNIX_TIME_TO_MJD(t) ( MJD_1970_01_01 + ((t) / 86400.0) )   // NOLINT
#define MJD_TO_UNIX_TIME(m) ( (long) (((m) - MJD_1970_01_01) * 86400.0) )   // NOLINT
#define STRUCT_TM_INIT {0, 0, 0, 0, 0, 0, 0, 0}

namespace ska::pst::common {

  /**
   * @brief Provides functionality for time stamps and conversion between different formats.
   *
   */
  class Time {

    public:

      /**
       * @brief Construct a new Time object initialized to the unix epoch
       *
       */
      Time() = default;

      /**
       * @brief Construct a new Time object from a timestamp
       *
       * @param timestamp timestamp in form YYYY-MM-DD-HH:MM:SS +tz
       */
      explicit Time(const char * timestamp);

      /**
       * @brief Construct a new Time object from a time epoch
       *
       * @param timestamp timestamp to initalize with
       */
      explicit Time(time_t timestamp);

      /**
       * @brief Destroy the Time object
       *
       */
      ~Time() = default;

      /**
       * @brief Set the time of the object
       *
       * @param timestamp in form YYYY-MM-DD-HH:MM:SS +tz
       */
      void set_time(const std::string &timestamp);

      /**
       * @brief Set the time of the object
       *
       * @param timestamp in form YYYY-MM-DD-HH:MM:SS +tz
       */
      void set_time(time_t timestamp) { epoch = timestamp; };

      /**
       * @brief Return the time of the object
       *
       * @return time_t epoch to return
       */
      auto get_time() -> time_t { return epoch; };

      /**
       * @brief Set the fractional seconds part of the timestamp
       *
       * @param seconds of the fractional part of the timestamp
       */
      void set_fractional_time(double seconds);

      /**
       * @brief Set the fractional seconds part of the timestamp
       *
       * @param attoseconds of the fractional part of the timestamp
       */
      void set_fractional_time(uint64_t attoseconds);

      /**
       * @brief Get the fractional time of the timestamp in seconds
       *
       * @return double fractional time in seconds
       */
      auto get_fractional_time() -> double;

      /**
       * @brief Get the fractional time of the timestamp in attoseconds
       *
       * @return double fractional time in attoseconds
       */
      auto get_fractional_time_attoseconds() -> uint64_t;

      /**
       * @brief Return the timestamp as a Modified Julian Day
       * The return value is an integer day
       * @return int
       */
      auto get_mjd_day() -> int { return static_cast<int>UNIX_TIME_TO_MJD (epoch); };

      /**
       * @brief Return the year of the object
       *
       * @return int
       */
      auto get_gm_year() -> int;

      /**
       * @brief Return the month of the object
       *
       * @return int
       */
      auto get_gm_month() -> int;

      /**
       * @brief Convert an MJD to the time_t
       *
       * @param mjd
       * @return time_t
       */
      static auto mjd2utctm(double mjd) -> time_t;

      /**
       * @brief Add the specified seconds to the object
       *
       * @param n number of seconds to add
       */
      void add_seconds(unsigned n) { epoch += n; };

      /**
       * @brief Subtract the specified seconds from the object
       *
       * @param n number of seconds to subtract
       */
      void sub_seconds(unsigned n) { epoch -= n; };

      /**
       * @brief Return a string representation of the timestamp in the local time zone
       *
       * @return std::string localtime timestamp in YYYY-MM-DD-HH:MM:SS format
       */
      auto get_localtime() -> std::string;

      /**
       * @brief Return a string representation of the timestamp in the UTC time zone
       *
       * @return std::string gmtime timestamp in YYYY-MM-DD-HH:MM:SS format
       */
      auto get_gmtime() -> std::string;

      /**
       * @brief Convert the provided epoch into a string timestamp in the local time zone
       *
       * @param e epoch to convert
       * @return std::string timestamp in local time zone
       */
      static auto format_localtime(time_t e) -> std::string;

      /**
       * @brief Converted the provided epoch into a string timestamp in the UTC time zone
       *
       * @param e epoch to convert
       * @return std::string timestamp in the UTC time zone
       */
      static auto format_gmtime(time_t e) -> std::string;

    private:

      /**
       * @brief time stamp relative to unix epoch
       *
       */
      time_t epoch{0};

      /**
       * @brief fractional component of the timestamp in attoseconds
       *
       */
      uint64_t attoseconds{0};
  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_Time_h
