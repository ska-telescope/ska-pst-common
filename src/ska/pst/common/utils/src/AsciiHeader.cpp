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

#include <cstring>                           // for size_t, strlen
#include <map>                               // for map
#include <string>                            // for string, allocator, getline
#include <utility>                           // for pair
#include <vector>                            // for vector
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/AsciiHeader.h"

ska::pst::common::AsciiHeader::AsciiHeader()
{
  header_size = DEFAULT_HEADER_SIZE;
  params.resize(0);
}

ska::pst::common::AsciiHeader::AsciiHeader(size_t nbytes)
{
  header_size = nbytes;
  params.resize(0);
}

ska::pst::common::AsciiHeader::AsciiHeader(const ska::pst::common::AsciiHeader &obj)
{
  header_size = obj.get_header_size();
  params = obj.params;
}

void ska::pst::common::AsciiHeader::clone(const ska::pst::common::AsciiHeader &obj)
{
  header_size = obj.get_header_size();
  params = obj.params;
}

void ska::pst::common::AsciiHeader::clone_stream(const ska::pst::common::AsciiHeader &obj, unsigned stream)
{
  header_size = obj.get_header_size();
  params = obj.params;

  // check each key, renaming any KEY_stream to KEY and removing KEY_otherstream
  std::vector<std::string> keys = header_get_keys();

  std::string the_suffix = "_" + std::to_string(stream);
  std::regex any_stream("(.*)_[0-9]+");
  std::regex the_stream("(.*)" + the_suffix);
  for (auto & key : keys)
  {
    if (std::regex_match(key, any_stream))
    {
      if (std::regex_match(key, the_stream))
      {
        std::string new_key = key.substr(0, key.size() - the_suffix.size());
        set_val(new_key, get_val(key));
      }
      del(key);
    }
  }
}

void ska::pst::common::AsciiHeader::append_header(const ska::pst::common::AsciiHeader &obj)
{
  for (auto & param : obj.params)
  {
    set_val(param.first, param.second);
  }
}

auto ska::pst::common::AsciiHeader::raw() const -> std::string
{
  std::string output;
  for (auto & param : params)
  {
    std::ostringstream line;
    const auto key_length = uint32_t(param.first.size());
    int min_width = std::max(key_padding, key_length + 1);
    line << std::left << std::setw(min_width) << param.first << param.second << std::endl;
    output.append(line.str());
  }
  return output;
}

void ska::pst::common::AsciiHeader::resize(size_t new_size)
{
  header_size = new_size;
}

auto ska::pst::common::AsciiHeader::get_header_size() const -> size_t
{
  return header_size;
}

auto ska::pst::common::AsciiHeader::get_header_length() const -> size_t
{
  return size_t(raw().length());
}

void ska::pst::common::AsciiHeader::load_from_file(const std::string &filename)
{
  spdlog::debug("ska::pst::common::AsciiHeader::load_from_file filename={}", filename);
  std::ifstream header_file(filename);
  if (!header_file.good())
  {
    spdlog::error("ska::pst::common::AsciiHeader::load_from_file could not open file for reading");
    throw std::runtime_error("ska::pst::common::AsciiHeader::load_from_file could not open file for reading");
  }

  std::string line;
  while (std::getline(header_file, line))
  {
    load_from_line(line);
  }
}

void ska::pst::common::AsciiHeader::load_from_str(const char * raw_header)
{
  std::stringstream ss(raw_header);
  std::string line;
  while (std::getline(ss, line, '\n'))
  {
    load_from_line(line);
  }
}

void ska::pst::common::AsciiHeader::load_from_string(const std::string &raw_header)
{
  std::stringstream ss(raw_header);
  std::string line;
  while (std::getline(ss, line, '\n'))
  {
    load_from_line(line);
  }
}

void ska::pst::common::AsciiHeader::load_from_line(const std::string &line)
{
  std::istringstream iss(line);
  std::string key, value;
  iss >> key >> value;
  // ignore keys starting with #
  if (key[0] == '#' || key.size() == 0 || value.size() == 0)
  {
    return;
  }
  spdlog::trace("ska::pst::common::AsciiHeader::load_from_line set_val({}, {})", key, value);
  set_val(key, value);
}

void ska::pst::common::AsciiHeader::append_from_str(const char * raw_header)
{
  load_from_str(raw_header);
}

void ska::pst::common::AsciiHeader::del(const std::string &key)
{
  check_not_empty(key);
  for (auto it=params.begin(); it != params.end(); it++)
  {
    if ((*it).first == key)
    {
      // Notice that the iterator is decremented after it is passed
      // to erase() but before erase() is executed
      params.erase(it--);
    }
  }
}

auto ska::pst::common::AsciiHeader::has(const std::string &key) const -> bool
{
  check_not_empty(key);
  for (auto & param : params)
  {
    if (param.first == key)
    {
      return true;
    }
  }
  return false;
}

auto ska::pst::common::AsciiHeader::get_size(const char * filename) -> size_t
{
  size_t hdr_size = -1;
  AsciiHeader tmp;
  tmp.load_from_file(filename);
  tmp.get("HDR_SIZE", &hdr_size);
  return hdr_size;
}

auto ska::pst::common::AsciiHeader::get_val(const std::string &key) const -> std::string
{
  check_not_empty(key);
  for (auto & param : params)
  {
    if (param.first == key)
    {
      return param.second;
    }
  }
  throw std::runtime_error("ska::pst::common::AsciiHeader::get_val key [" + key + "] did not exist");
}

auto ska::pst::common::AsciiHeader::get_uint32(const std::string &key) const -> uint32_t
{
    uint32_t value{0};
    get(key, &value);
    return value;
}

auto ska::pst::common::AsciiHeader::get_int32(const std::string &key) const -> int32_t
{
    int32_t value{0};
    get(key, &value);
    return value;
}

auto ska::pst::common::AsciiHeader::get_uint64(const std::string &key) const -> uint64_t
{
    uint64_t value{0};
    get(key, &value);
    return value;
}

auto ska::pst::common::AsciiHeader::get_float(const std::string &key) const -> float
{
    float value{0.0};
    get(key, &value);
    return value;
}

auto ska::pst::common::AsciiHeader::get_double(const std::string &key) const -> double
{
    double value{0.0};
    get(key, &value);
    return value;
}

void ska::pst::common::AsciiHeader::set_val(const std::string &key, const std::string &val)
{
  check_not_empty(key);
  check_not_empty(val);
  bool found = false;
  for (auto & param : params)
  {
    if (param.first == key)
    {
      param.second = val;
      found = true;
    }
  }
  if (!found)
  {
    params.emplace_back(std::make_pair(key, val));
  }
}

auto ska::pst::common::AsciiHeader::header_get_keys() const -> std::vector<std::string>
{
  std::vector<std::string> result;
  for (auto & param : params)
  {
    result.emplace_back(param.first);
  }
  return result;
}

void ska::pst::common::AsciiHeader::set_key_padding(uint32_t to_pad)
{
  uint32_t min_pad = 1;
  key_padding = std::max(min_pad, to_pad);
}

auto ska::pst::common::AsciiHeader::get_key_padding() const -> uint32_t
{
  return key_padding;
}

void ska::pst::common::AsciiHeader::check_not_empty(const std::string &str) const
{
  if (str.length() == 0)
  {
    throw std::runtime_error("ska::pst::common::AsciiHeader::check_not_empty str was zero length");
  }
  if (str.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
  {
    throw std::runtime_error("ska::pst::common::AsciiHeader::check_not_empty str contained only whitespace");
  }
}

auto ska::pst::common::AsciiHeader::compute_bits_per_sample() const -> unsigned
{
  unsigned nchan{}, nbit{}, npol{}, nbin{}, ndim{};
  get("NCHAN", &nchan);
  get("NBIT", &nbit);
  get("NPOL", &npol);
  get("NDIM", &ndim);
  return (nchan * nbit * npol * ndim);
}

auto ska::pst::common::AsciiHeader::compute_bytes_per_second() const -> double
{
  static constexpr double nbits_per_byte = 8.0;
  static constexpr double microseconds_per_second = 1000000;

  double tsamp{};
  get("TSAMP", &tsamp);

  auto nbit_per_sample = double(compute_bits_per_sample());
  double nsamp_per_second =  microseconds_per_second / tsamp;
  double nbit_per_second = nbit_per_sample * nsamp_per_second;
  double bytes_ps = nbit_per_second / nbits_per_byte;

  spdlog::debug("ska::pst::common::AsciiHeader::compute_bytes_per_second bytes_ps={}", bytes_ps);
  return bytes_ps;
}

template void ska::pst::common::AsciiHeader::get<int32_t>(const std::string &key, int32_t * val);
template void ska::pst::common::AsciiHeader::get<int32_t>(const std::string &key, int32_t * val) const;
template void ska::pst::common::AsciiHeader::get<uint32_t>(const std::string &key, uint32_t * val) const;
template void ska::pst::common::AsciiHeader::get<int64_t>(const std::string &key, int64_t * val) const;
template void ska::pst::common::AsciiHeader::get<uint64_t>(const std::string &key, uint64_t * val) const;
template void ska::pst::common::AsciiHeader::get<float>(const std::string &key, float * val) const;
template void ska::pst::common::AsciiHeader::get<double>(const std::string &key, double * val) const;