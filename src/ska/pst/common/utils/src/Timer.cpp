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

#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/Timer.h"

ska::pst::common::Timer::Timer()
{
  reset();
}

void ska::pst::common::Timer::reset()
{
  gettimeofday(&start_epoch, nullptr);
  target = 0;
}

void ska::pst::common::Timer::wait_until(double offset)
{
  target += offset;
  while (get_elapsed_microseconds() < target)
  {
    ;
  }
}

auto ska::pst::common::Timer::get_elapsed_microseconds() -> double
{
  static constexpr double microseconds_per_second = 1000000;
  struct timeval timestamp{};
  gettimeofday(&timestamp, nullptr);
  const time_t seconds = timestamp.tv_sec - start_epoch.tv_sec;
  const suseconds_t usec =  timestamp.tv_usec - start_epoch.tv_usec;
  return ((static_cast<double>(seconds) * microseconds_per_second) + static_cast<double>(usec));
}

auto ska::pst::common::Timer::get_elapsed_milliseconds() -> int
{
  static constexpr double microseconds_per_millisecond = 1000;
  return static_cast<int>(rint(get_elapsed_microseconds()/microseconds_per_millisecond));
}

void ska::pst::common::Timer::print_rates(uint64_t bytes)
{
  static constexpr double microseconds_per_second = 1000000;
  static constexpr double bytes_per_gigabyte = 1073741824;
  double elapsed_microseconds = get_elapsed_microseconds();
  double bytes_per_second = 0;
  double gbytes_per_second = 0;
  if (elapsed_microseconds > 0)
  {
    bytes_per_second = static_cast<double>(bytes) / (elapsed_microseconds / microseconds_per_second);
    gbytes_per_second = bytes_per_second / bytes_per_gigabyte;
  }

  SPDLOG_INFO("Data: {} bytes", bytes);
  SPDLOG_INFO("Duration: {} microseconds", elapsed_microseconds);
  SPDLOG_INFO("Rate: {} GB/s ({} B/s)", gbytes_per_second, bytes_per_second);
}