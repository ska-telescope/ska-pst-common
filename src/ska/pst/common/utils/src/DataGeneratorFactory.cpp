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

#include "ska/pst/common/utils/DataGeneratorFactory.h"
#include "ska/pst/common/utils/RandomDataGenerator.h"
#include "ska/pst/common/utils/SineWaveGenerator.h"

auto ska::pst::common::get_supported_data_generators() -> std::vector<std::string>
{
  std::vector<std::string> supported;
  supported.emplace_back("Random");
  return supported;
}

auto ska::pst::common::get_supported_data_generators_list() -> std::string
{
  std::vector<std::string> supported = ska::pst::common::get_supported_data_generators();
  std::string delim = ", ";
  return std::accumulate(supported.begin() + 1, supported.end(), supported[0],
    [&delim](const std::string& x, const std::string& y) {
      return x + delim + y;
    }
  );
}

auto ska::pst::common::DataGeneratorFactory(const std::string &name) -> std::shared_ptr<ska::pst::common::DataGenerator>
{
  if (name == "Random") {
    return std::shared_ptr<ska::pst::common::DataGenerator>(new ska::pst::common::RandomDataGenerator());
  } 
  if (name == "Sine") {
    return std::shared_ptr<ska::pst::common::DataGenerator>(new ska::pst::common::SineWaveGenerator());
  }

  throw std::runtime_error("ska::pst::common::DataGeneratorFactory unrecognized name");  
}

