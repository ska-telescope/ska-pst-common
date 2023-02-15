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

#ifndef SKA_PST_COMMON_definitions_h
#define SKA_PST_COMMON_definitions_h

namespace ska::pst::common
{
  //! number of micro seconds in a second
  static constexpr double microseconds_per_second = 1e6;

  //! number of bits in a byte
  static constexpr unsigned bits_per_byte = 8;

  //! number of giga bits (10^9 bits) in a byte
  static constexpr double gigabits_per_byte = double(bits_per_byte) / double(1e9);

  //! number of deciseconds in a second
  static constexpr unsigned deciseconds_per_second = 10;

  //! number of micro seconds in a decisecond
  static constexpr unsigned microseconds_per_decisecond = 100000;

  //! number of milli seconds in a second
  static constexpr int milliseconds_per_second = 1000;

  //! number of microseconds in a millisecond
  static constexpr int microseconds_per_millisecond = 1000;

  //! number of attoseconds in a picosecond
  static constexpr int attoseconds_per_picosecond = 1000;

  //! value of 100 milliseconds
  static constexpr int hundred_milliseconds = 100;

  //! number of millihertz (10^-3) in a megahertz (10^6)
  static constexpr unsigned millihertz_per_megahertz = 1e9;

  //! number of attoseconds (10^-18) in a micro second (10^-6)
  static constexpr double attoseconds_per_microsecond = 1e12;

  //! number of attoseconds (10^-18) in a second
  static constexpr uint64_t attoseconds_per_second = 1e18;

  //! minimum resolution (i.e. block size) for UDP packet weights
  static constexpr unsigned packet_weights_resolution = 8; // bytes

  //! default kernel socket receive buffer size in bytes
  static constexpr size_t default_kernel_socket_bufsz = 131071; // 128 kB - 1 byte

  //! size of the kernel socket receive buffer in bytes
  static constexpr size_t ideal_kernel_socket_bufsz = 67108864; // 64 MB

  //! size of the kernel socket receive buffer in bytes
  static constexpr size_t receive_buffer_size = 33554432; // 32 MB

  //! number of percentiles in 100 percent
  static constexpr float percentiles_per_100 = 100;

  // number of seconds in a microsecond
  static constexpr double seconds_per_microseconds = 0.000001;

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_definitions_h