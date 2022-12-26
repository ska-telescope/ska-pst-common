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

#include <cstdint>
#include <cstring>
#include <endian.h>

#ifndef SKA_PST_SMRB_UTILS_Endian_h
#define SKA_PST_SMRB_UTILS_Endian_h

namespace ska {
namespace pst {
namespace common {
namespace detail {

/**
 * @brief Template class to performs conversions between host byte order and big endian order for scalar values
 *
 * @tparam T type of scalar value to be converted
 */
template<typename T>
struct Endian
{
};

/**
 * @brief Perform conversions between host byte ordering and big endian for unsigned 64-bit integers
 *
 * @tparam unsigned 64-bit integer
 */
template<>
struct Endian<uint64_t>
{
  /**
   * @brief Convert host byte order input to big endian output
   *
   * @param in host byte ordered input value
   * @return uint64_t big endian representation of the input value
   */
  static uint64_t htobe(uint64_t in)
  {
    return htobe64(in);
  }

  /**
   * @brief Convert big endian input to host byte order output
   *
   * @param in big endian input value
   * @return uint64_t host byte ordered representation of the input value
   */
  static uint64_t betoh(uint64_t in)
  {
    return be64toh(in);
  }
};

/**
 * @brief Perform conversions between host byte ordering and big endian for unsigned 32-bit integers
 *
 * @tparam unsigned 32-bit integer
 */
template<>
struct Endian<std::uint32_t>
{
  /**
   * @brief Convert host byte order input to big endian output
   *
   * @param in host byte ordered input value
   * @return std::uint32_t big endian representation of the input value
   */
  static std::uint32_t htobe(std::uint32_t in)
  {
    return htobe32(in);
  }

  /**
   * @brief Convert big endian input to host byte order output
   *
   * @param in big endian input value
   * @return std::uint32_t host byte ordered representation of the input value
   */
  static std::uint32_t betoh(std::uint32_t in)
  {
    return be32toh(in);
  }
};

/**
 * @brief Perform conversions between host byte ordering and big endian for unsigned 16-bit integers
 *
 * @tparam unsigned 16-bit integer
 */
template<>
struct Endian<std::uint16_t>
{
  /**
   * @brief Convert host byte order input to big endian output
   *
   * @param in host byte ordered input value
   * @return std::uint16_t big endian representation of the input value
   */
  static std::uint16_t htobe(std::uint16_t in)
  {
    return htobe16(in);
  }

  /**
   * @brief Convert big endian input to host byte order output
   *
   * @param in big endian input value
   * @return std::uint16_t host byte ordered representation of the input value
   */
  static std::uint16_t betoh(std::uint16_t in)
  {
    return be16toh(in);
  }
};

} // namespace detail

/**
 * @brief Perform conversion of a scalar value from host byte order to big endian order.
 *
 * @tparam T type of scalar to convert
 * @param in host byte order input value to convert
 * @return T big endian representation of in
 */
template<typename T>
static inline T htobe(T in)
{
  return detail::Endian<T>::htobe(in);
}

/**
 * @brief Perform conversion of a scalar from big endian to host byte order
 *
 * @tparam T type of the scalar value to convert
 * @param in big endian input value to convert
 * @return T host byte over representation of in
 */
template<typename T>
static inline T betoh(T in)
{
  return detail::Endian<T>::betoh(in);
}

} // common
} // pst
} // ska

#endif // SKA_PST_SMRB_UTILS_Endian_h
