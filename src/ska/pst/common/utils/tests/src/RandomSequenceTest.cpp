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
#include <filesystem>
#include <vector>

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/tests/RandomSequenceTest.h"

auto main(int argc, char* argv[]) -> int
{
  spdlog::set_level(spdlog::level::debug);
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

RandomSequenceTest::RandomSequenceTest()
    : ::testing::Test()
{
}

void RandomSequenceTest::SetUp()
{
  header.load_from_file(test_data_file("data_header.txt"));
  buffer.resize(default_buffer_size);
}

void RandomSequenceTest::TearDown()
{
}

TEST_F(RandomSequenceTest, test_generate_validate) // NOLINT
{
  RandomSequence sg;

  auto buffer_ptr = reinterpret_cast<uint8_t*>(&buffer[0]);

  sg.configure(header);
  sg.generate(buffer_ptr, buffer.size());
  sg.reset();
  EXPECT_TRUE(sg.validate(buffer_ptr, buffer.size()));
  EXPECT_FALSE(sg.validate(buffer_ptr, buffer.size()));

  // perform a shift of all the values in the buffer
  for (unsigned i=0; i<buffer.size()-1; i++)
  {
    buffer_ptr[i] = buffer_ptr[i+1]; // NOLINT
  }

  sg.reset();
  EXPECT_FALSE(sg.validate(buffer_ptr, buffer.size()));
}

TEST_F(RandomSequenceTest, test_seek) // NOLINT
{
  RandomSequence sg;
  auto buffer_ptr = reinterpret_cast<uint8_t*>(&buffer[0]);

  sg.configure(header);
  sg.generate(buffer_ptr, buffer.size());
  sg.reset();

  size_t half_buffer = buffer.size() / 2;
  sg.seek(half_buffer);
  EXPECT_TRUE(sg.validate(buffer_ptr + half_buffer, half_buffer)); // NOLINT
}

} // namespace ska::pst::common::test
