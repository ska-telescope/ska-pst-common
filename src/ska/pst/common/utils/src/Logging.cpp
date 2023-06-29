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

#include <algorithm>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/Logging.h"

void ska::pst::common::setup_spdlog() {
    spdlog::logger * logger = spdlog::default_logger_raw();
    logger->set_pattern(SKA_LOGGING_FORMAT, spdlog::pattern_time_type::utc);
}

spdlog::level::level_enum ska::pst::common::get_spdlog_level(ska::pst::lmc::LogLevel level)
{
    bool found = (ska::pst::common::log_level_map.find(level) != log_level_map.end());
    if (!found)
    {
        throw std::runtime_error("ska::pst::common::get_spdlog_level lmclog level did not map to spdlog level");
    }
    return ska::pst::common::log_level_map[level];
}

ska::pst::lmc::LogLevel ska::pst::common::get_lmclog_level(spdlog::level::level_enum level)
{
    bool found = false;
    ska::pst::lmc::LogLevel mapped_level;
    std::for_each(log_level_map.begin(), log_level_map.end(),
        [&level, &mapped_level, &found](const std::pair<ska::pst::lmc::LogLevel, spdlog::level::level_enum> &p) {
            if (p.second == level)
            {
                found = true;
                mapped_level = p.first;
            }
        }
    );
    if (!found)
    {
        throw std::runtime_error("ska::pst::common::get_lmclog_level spdlog level did not map to lmclog level");
    }
    return mapped_level;
}
