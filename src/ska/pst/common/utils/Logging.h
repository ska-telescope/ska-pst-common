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

#include <map>
#include <spdlog/spdlog.h>

#include "ska/pst/lmc/ska_pst_lmc.pb.h"

#ifndef SKA_PST_COMMON_UTILS_Logging_h
#define SKA_PST_COMMON_UTILS_Logging_h

#define SKA_LOGGING_FORMAT "1|%Y-%m-%dT%T.%eZ|%l|Thread-%t|%!|%s#%#||%v"

namespace ska::pst::common {

  static std::map<ska::pst::lmc::LogLevel, spdlog::level::level_enum> log_level_map
  {
    { ska::pst::lmc::LogLevel::INFO, spdlog::level::info },
    { ska::pst::lmc::LogLevel::DEBUG, spdlog::level::debug},
    { ska::pst::lmc::LogLevel::WARNING, spdlog::level::warn},
    { ska::pst::lmc::LogLevel::CRITICAL, spdlog::level::critical},
    { ska::pst::lmc::LogLevel::ERROR, spdlog::level::err},
  };

  /**
   * Used to set up the spdlog logging framework
   */
  void setup_spdlog();

  /**
   * @brief Get the spdlog level corresponding to a lmclog level
   *
   * @param level lmclog level
   * @return spdlog::level::level_enum mapped spdlog level
   */
  spdlog::level::level_enum get_spdlog_level(ska::pst::lmc::LogLevel level);

  /**
   * @brief Get the lmclog level corresponding to a spdlog level
   *
   * @param level spdlog level
   * @return ska::pst::lmc::LogLevel mapped lmclog level
   */
  ska::pst::lmc::LogLevel get_lmclog_level(spdlog::level::level_enum level);

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTILS_Logging_h

