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

#include "ska/pst/common/utils/FileBlockLoader.h"

#include <spdlog/spdlog.h>
#include <sys/mman.h> // mmap and munmap

ska::pst::common::FileBlockLoader::FileBlockLoader(const std::string& file_path)
	: reader(new FileReader(file_path))
{
  ssize_t hdr_size = reader->read_header();
  ssize_t data_size = reader->get_file_size()-hdr_size;

  map = mmap(0, data_size, PROT_READ, MAP_SHARED, reader->_get_fd(), hdr_size);
  if (map == MAP_FAILED)
  {
    SPDLOG_ERROR("ska::pst::common::FileBlockLoader::ctor failed mmap fd={}", reader->_get_fd());
    throw std::runtime_error("ska::pst::common::FileReader::ctor failed mmap");
  }

  block_info.first = reinterpret_cast<char*>(map);
  block_info.second = data_size;
}

ska::pst::common::FileBlockLoader::~FileBlockLoader ()
{
  if (munmap(map, reader->get_file_size()) == -1)
  {
    SPDLOG_ERROR("ska::pst::common::FileBlockLoader::ctor failed munmap");
    throw std::runtime_error("ska::pst::common::FileReader::dtor failed munmap");
  }
}

const ska::pst::common::AsciiHeader& ska::pst::common::FileBlockLoader::get_header() const
{
  return reader->get_header();
}

std::pair<char*,size_t> ska::pst::common::FileBlockLoader::next_block()
{
  auto result = block_info;
  block_info = std::pair<char*,size_t> (nullptr, 0); // next call to next_block returns EOD marker
  return result;
}

