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

#include <sstream>

#include "ska/pst/common/utils/ValidationContext.h"
#include "ska/pst/common/statemodel/StateModelException.h"

template <typename T>
void ska::pst::common::ValidationContext::add_validation_error(
  const std::string& field_name,
  T value,
  const std::string& message
)
{
  std::ostringstream out;
  out.precision(20);
  out << value;

  auto str_val = out.str();

  ska::pst::common::validation_error_record_t err{
    field_name, str_val, message
  };

  errors.push_back(err);
}

void ska::pst::common::ValidationContext::throw_error_if_not_empty()
{
  if (errors.empty()) {
    return;
  }

  auto first = true;
  std::stringstream ss;
  for (auto & err : errors)
  {
    if (first) {
      first = false;
    } else {
      ss << '\n';
    }
    ss << err.field_name << " with value ";
    ss << err.value << " failed validation: ";
    ss << err.message;
  }

  throw ska::pst::common::pst_validation_error(ss.str());
}


template void ska::pst::common::ValidationContext::add_validation_error<std::string>(const std::string &field_name, std::string value, const std::string &message);
template void ska::pst::common::ValidationContext::add_validation_error<int32_t>(const std::string &field_name, int32_t value, const std::string &message);
template void ska::pst::common::ValidationContext::add_validation_error<uint32_t>(const std::string &field_name, uint32_t value, const std::string &message);
template void ska::pst::common::ValidationContext::add_validation_error<int64_t>(const std::string &field_name, int64_t value, const std::string &message);
template void ska::pst::common::ValidationContext::add_validation_error<uint64_t>(const std::string &field_name, uint64_t value, const std::string &message);
template void ska::pst::common::ValidationContext::add_validation_error<float>(const std::string &field_name, float value, const std::string &message);
template void ska::pst::common::ValidationContext::add_validation_error<double>(const std::string &field_name, double value, const std::string &message);
