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

#include "ska/pst/common/utils/FileBlockProducer.h"

#include <gtest/gtest.h>

#ifndef SKA_PST_COMMON_UTILS_TESTS_FileBlockProducerTest_h
#define SKA_PST_COMMON_UTILS_TESTS_FileBlockProducerTest_h

namespace ska::pst::common::test {

  /**
   * @brief Test the FileBlockProducer class
   * 
   * @details
   * 
   */
  class FileBlockProducerTest : public ::testing::Test
  {
    protected:
      void SetUp() override;

      void TearDown() override;

    public:
      FileBlockProducerTest();

      ~FileBlockProducerTest();

      ska::pst::common::AsciiHeader header;

      std::vector<char> file_header;

      std::vector<char> file_data;

      std::string file_name{"/tmp/FileBlockProducerTest.dada"};

      uint32_t header_size{0};

      uint32_t data_size{1048576};

    private:

  };

} // namespace ska::pst::common::test

#endif // SKA_PST_COMMON_UTILS_TESTS_FileBlockProducerTest_h

