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

#include <algorithm> // for std::replace
#include <fstream>
#include <spdlog/spdlog.h>

#include "ska/pst/common/version.h"
#include "ska/pst/common/tests/versionTest.h"
#include "ska/pst/common/testutils/GtestMain.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test
{

void versionTest::SetUp()
{
  // read the version from the release file in the top level directory
  std::string filename = test_data_file(".release");
  SPDLOG_DEBUG("ska::pst::common::test::versionTest::SetUp filename={}", filename);
  std::ifstream release_file(filename);
  if (!release_file.good())
  {
    SPDLOG_ERROR("ska::pst::common::test::versionTest::SetUp could not open file for reading");
    throw std::runtime_error("ska::pst::common::test::versionTest::SetUp could not open file for reading");
  }

  std::string line;
  std::string search_prefix = "release=";
  while (std::getline(release_file, line))
  {
    if (line.rfind(search_prefix, 0) == 0)
    {
      release_version = line.substr(search_prefix.size());
      SPDLOG_DEBUG("ska::pst::common::test::versionTest::SetUp raw release_version={}", release_version);
    }
  }
  release_file.close();
  for (unsigned i=0; i<release_version.size(); i++)
  {
    if (release_version[i] == '.')
    {
      release_version[i] = ':';
    }
  }
  SPDLOG_DEBUG("ska::pst::common::test::versionTest::SetUp converted release_version={}", release_version);
}

void versionTest::TearDown()
{
}

TEST_F(versionTest, test_version) // NOLINT
{
  std::string library_version = ska::pst::common::get_version_string();
  ASSERT_EQ(library_version, release_version);
}

} // namespace ska::pst::common::test