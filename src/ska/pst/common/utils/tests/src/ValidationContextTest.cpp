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

#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/tests/ValidationContextTest.h"
#include "ska/pst/common/utils/ValidationContext.h"
#include "ska/pst/common/statemodel/StateModelException.h"
#include "ska/pst/common/testutils/GtestMain.h"

auto main(int argc, char* argv[]) -> int
{
  return ska::pst::common::test::gtest_main(argc, argv);
}

namespace ska::pst::common::test {

ValidationContextTest::ValidationContextTest()
    : ::testing::Test()
{
}

void ValidationContextTest::SetUp()
{
}

void ValidationContextTest::TearDown()
{
}

TEST_F(ValidationContextTest, default_constructor) // NOLINT
{
  ValidationContext context;
  EXPECT_TRUE(context.is_empty()); // NOLINT
}

TEST_F(ValidationContextTest, throw_error_if_not_empty) // NOLINT
{
  ValidationContext context;

  context.add_validation_error<int32_t>("DATA_PORT", 42, "Invalid data port"); // NOLINT
  try {
    context.throw_error_if_not_empty();
    FAIL(); // NOLINT
  } catch (ska::pst::common::pst_validation_error& exc) {
    EXPECT_EQ("DATA_PORT with value 42 failed validation: Invalid data port", std::string(exc.what())); // NOLINT
  }
}

TEST_F(ValidationContextTest, throw_error_if_not_empty_multiple_errors) // NOLINT
{
  ValidationContext context;

  context.add_validation_error<int32_t>("DATA_PORT", 42, "Invalid data port"); // NOLINT
  context.add_validation_error<std::string>("SOURCE", "Not the right source", "invalid source");
  try {
    context.throw_error_if_not_empty();
    FAIL(); // NOLINT
  } catch (ska::pst::common::pst_validation_error& exc) {
    EXPECT_EQ( "DATA_PORT with value 42 failed validation: Invalid data port\nSOURCE with value Not the right source failed validation: invalid source", std::string(exc.what())); // NOLINT
  }
}

TEST_F(ValidationContextTest, add_missing_field_error) // NOLINT
{
  ValidationContext context;

  context.add_missing_field_error("SOURCE_ID");

  try {
    context.throw_error_if_not_empty();
    FAIL(); // NOLINT
  } catch (ska::pst::common::pst_validation_error& exc) {
    EXPECT_EQ( "SOURCE_ID with value <none> failed validation: required value missing", std::string(exc.what())); // NOLINT
  }
}

TEST_F(ValidationContextTest, copy_errors) // NOLINT
{
  ValidationContext context1;
  ValidationContext context2;

  context1.add_missing_field_error("SOURCE_ID");
  ASSERT_FALSE(context1.is_empty()); // NOLINT
  ASSERT_TRUE(context2.is_empty()); // NOLINT

  context2.copy_errors(context1);
  ASSERT_FALSE(context2.is_empty()); // NOLINT

  ASSERT_THROW(context2.throw_error_if_not_empty(), ska::pst::common::pst_validation_error); // NOLINT
}

} // namespace ska::pst::common::test