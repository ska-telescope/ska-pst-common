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

#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/Time.h"

ska::pst::common::Time::Time(const char * s)
{
  set_time(s);
}

ska::pst::common::Time::Time(time_t now) :
  epoch(now)
{
}

void ska::pst::common::Time::set_time(const std::string &timestamp)
{
  std::istringstream iss;
  iss.str(std::string(timestamp));
  std::string token;
  char delim = '.';

  // check for YYYY-MM-DD-HH:MM:SS.ssss
  if (std::getline(iss, token, delim))
  {
    std::string utc_string;
    utc_string.append(token);
    utc_string.append(" UTC");
    const char * format = "%Y-%m-%d-%H:%M:%S %Z";
    struct tm time_tm = STRUCT_TM_INIT;
    char * first_unprocessed = strptime(utc_string.c_str(), format, &time_tm);
    if (first_unprocessed == nullptr)
    {
      epoch = 0;
      std::cerr << "Warning: "<< timestamp << " not a valid timestamp, using " << get_gmtime() << " instead" << std::endl;
    }
    else
    {
      epoch = timegm(&time_tm);
    }
  }

  if (std::getline(iss, token, delim))
  {
    int num_decimal = token.size();
    static constexpr int base10 = 10;
    double seconds_conversion = pow(double(base10), num_decimal);
    try
    {
      double fractional_seconds = double(std::stoi(token)) / seconds_conversion;
      set_fractional_time(fractional_seconds);
    }
    catch  (std::exception& exc)
    {
      attoseconds = 0;
    }
  }
}

auto ska::pst::common::Time::mjd2utctm(double mjd) -> time_t
{
  static constexpr int seconds_in_day = 86400;
  static constexpr int seconds_per_minute = 60;
  static constexpr int minutes_per_hour = 60;
  static constexpr int seconds_per_hour = minutes_per_hour * seconds_per_minute;
  static constexpr int julian_day_epoch = 2400001;

  auto days = int(mjd);
  double fdays = mjd - double(days);
  double seconds = fdays * double(seconds_in_day);
  auto secs = int(seconds);
  double fracsec = seconds - double(secs);

  static constexpr int milliseconds_in_second = 1000;
  if (int(rint(fracsec*milliseconds_in_second)) >= milliseconds_in_second/2)
  {
    secs++;
  }

  int julian_day = days + julian_day_epoch;
  int n_four = 4  * (julian_day+((6*((4*julian_day-17918)/146097))/4+1)/2-37); // NOLINT
  int n_dten = 10 * (((n_four-237)%1461)/4) + 5; // NOLINT

  struct tm gregdate = STRUCT_TM_INIT;
  gregdate.tm_year = n_four/1461 - 4712 - 1900; // NOLINT extra -1900 for C struct tm
  gregdate.tm_mon  = (n_dten/306+2)%12;         // NOLINT struct tm mon 0->11
  gregdate.tm_mday = (n_dten%306)/10 + 1;       // NOLINT

  gregdate.tm_hour = secs / seconds_per_hour;
  secs -= seconds_per_hour * gregdate.tm_hour;

  gregdate.tm_min = secs / seconds_per_minute;
  secs -= seconds_per_minute * (gregdate.tm_min);

  gregdate.tm_sec = secs;

  gregdate.tm_isdst = -1;
  time_t date = mktime (&gregdate);

  return date;
}

auto ska::pst::common::Time::get_localtime() -> std::string
{
  return ska::pst::common::Time::format_localtime(epoch);
}

auto ska::pst::common::Time::get_gmtime() -> std::string
{
  return ska::pst::common::Time::format_gmtime(epoch);
}

auto ska::pst::common::Time::get_fractional_time() -> double
{
  return double(attoseconds) / attoseconds_per_second;
}

auto ska::pst::common::Time::get_fractional_time_attoseconds() -> uint64_t
{
  return attoseconds;
}

void ska::pst::common::Time::set_fractional_time(double fractional_seconds)
{
  if (fractional_seconds < 0)
  {
    throw std::runtime_error("ska::pst::common::Time::set_fractional_time seconds was < 0");
  }
  if (fractional_seconds >= 1.0)
  {
    throw std::runtime_error("ska::pst::common::Time::set_fractional_time seconds was >= 1");
  }
  double fractional_attoseconds = fractional_seconds * attoseconds_per_second;
  attoseconds = uint64_t(roundl(fractional_attoseconds));
}

void ska::pst::common::Time::set_fractional_time(uint64_t _attoseconds)
{
  if (_attoseconds >= attoseconds_per_second_u64)
  {
    throw std::runtime_error("ska::pst::common::Time::set_fractional_time attoseconds >= 1e18");
  }
  attoseconds = _attoseconds;
}

auto ska::pst::common::Time::format_localtime(time_t e) -> std::string
{
  std::ostringstream os;
  const char * format = "%Y-%m-%d-%H:%M:%S";
  struct tm * timeinfo = localtime(&e);
  os << std::put_time(timeinfo, format);
  return os.str();
}

auto ska::pst::common::Time::format_gmtime(time_t e) -> std::string
{
  std::ostringstream os;
  const char * format = "%Y-%m-%d-%H:%M:%S";
  struct tm * timeinfo = gmtime(&e);
  os << std::put_time(timeinfo, format);
  return os.str();
}

auto ska::pst::common::Time::get_gm_year() -> int
{
  struct tm * timeinfo = gmtime(&epoch);
  static constexpr int year_1900 = 1900;
  return timeinfo->tm_year + year_1900;
}

auto ska::pst::common::Time::get_gm_month() -> int
{
  struct tm * timeinfo = gmtime(&epoch);
  return timeinfo->tm_mon;
}
