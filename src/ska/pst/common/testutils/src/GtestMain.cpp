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

#include <string>
#include <spdlog/spdlog.h>

#include "ska/pst/common/testutils/GtestMain.h"
#include "ska/pst/common/utils/Logging.h"

namespace ska::pst::common::test {

auto test_data_dir() -> std::string&
{
    static std::string data_dir = ".";
    return data_dir;
}

auto test_data_file(std::string const& filename) -> std::string
{
    return test_data_dir() + "/" + filename;
}

auto gtest_main(int argc, char** argv) -> int
{
    // will process gtest options and pass on the rest
    testing::InitGoogleTest(&argc, argv);
    ska::pst::common::setup_spdlog();

    // process extra command line options;
    for (int i=0; i < argc; i++)
    {
        std::string const arg(argv[i]); // NOLINT
        if (arg == "--test_data")
        {
            if(++i < argc)
            {
                std::string const val(argv[i]); //NOLINT
                test_data_dir() = val;
            }
        }
        if (arg == "--debug")
        {
            if (spdlog::get_level() != spdlog::level::trace)
            {
                spdlog::set_level(spdlog::level::debug);
            }
        }
        if (arg == "--trace")
        {
            spdlog::set_level(spdlog::level::trace);
        }
    }

    return RUN_ALL_TESTS();
}

} // namespace ska::pst::common::test
