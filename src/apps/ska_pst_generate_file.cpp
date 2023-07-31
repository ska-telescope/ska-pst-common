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

#include "ska/pst/common/utils/FileWriter.h"
#include "ska/pst/common/utils/Logging.h"
#include "ska/pst/common/definitions.h"

#include <unistd.h>
#include <iostream>
#include <cfloat>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>

void usage();

auto main(int argc, char *argv[]) -> int
{
  ska::pst::common::setup_spdlog();

  bool use_o_direct = false;

  char verbose = 0;

  opterr = 0;

  int c = 0;

  while ((c = getopt(argc, argv, "hov")) != EOF)
  {
    switch(c)
    {
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
        break;

      case 'o':
        use_o_direct = true;
        break;

      case 'v':
        verbose++;
        break;

      default:
        std::cerr << "ERROR: unrecognised option: -" << char(optopt) << std::endl;
        usage();
        return EXIT_FAILURE;
        break;
    }
  }

  if (verbose > 0)
  {
    spdlog::set_level(spdlog::level::debug);
    if (verbose > 1)
    {
      spdlog::set_level(spdlog::level::trace);
    }
  }

  // Check arguments
  if ((argc - optind) != 3)
  {
    SPDLOG_ERROR("ERROR: 3 command line arguments are expected");
    usage();
    return EXIT_FAILURE;
  }

  std::string data_file(argv[optind]); // NOLINT
  std::string weights_file(argv[optind+1]); // NOLINT
  std::string output_dir(argv[optind+2]); // NOLINT
  std::string output_data_dir = output_dir + "/data";
  std::string output_weights_dir = output_dir + "/weights";

  int return_code = 0;

  try
  {

    ska::pst::common::FileWriter data_file_writer(use_o_direct);
    ska::pst::common::FileWriter weights_file_writer(use_o_direct);

    ssize_t data_bytes_remaining = 0; // requires calculation
    ssize_t weights_bytes_remaining = 0; // requires calculation

    const ska::pst::common::AsciiHeader data_header;
    const ska::pst::common::AsciiHeader weights_header;

    uint32_t data_size_factor = ska::pst::common::bits_per_float / data_header.get_uint32("NBIT");
    uint32_t unpacked_obs_offset = data_size_factor * data_header.get_uint32("OBS_OFFSET");
    double unpacked_bytes_per_second = data_size_factor * data_header.get_double("BYTES_PER_SECOND");
    uint32_t file_number = data_header.get_uint32("FILE_NUMBER");
    std::string utc_start = data_header.get_val("UTC_START");

    auto data_hdrsz = ssize_t(data_header.get_uint32("HDR_SIZE"));
    auto data_bufsz = ssize_t(data_header.get_uint32("RESOLUTION"));
    auto weights_bufsz = ssize_t(weights_header.get_uint32("RESOLUTION"));
    SPDLOG_DEBUG("ska_pst_generate_file RESOLUTION data={} weights={}", data_bufsz, weights_bufsz);

    // the RESOLUTION is tied to the number of time samples per UDP packet
    // for Low this is 32 * 207.36 us (approximately 6ms), increasing this by 64 for improved performance
    static constexpr uint32_t process_block_factor = 64;
    data_bufsz *= process_block_factor;
    weights_bufsz *= process_block_factor;

    std::filesystem::path output_data_path(output_data_dir);
    std::filesystem::path output_data_filename = output_data_path / data_file_writer.get_filename(utc_start, unpacked_obs_offset, file_number);
    SPDLOG_DEBUG("ska_pst_generate_file writing data to file {}", output_data_filename.generic_string());

    std::filesystem::path output_weights_path(output_weights_dir);
    std::filesystem::path output_weights_filename = output_weights_path / weights_file_writer.get_filename(utc_start, unpacked_obs_offset, file_number);
    SPDLOG_DEBUG("ska_pst_generate_file writing weights to file {}", output_weights_filename.generic_string());

    data_file_writer.open_file(output_data_filename);
    data_file_writer.write_header(data_header);

    weights_file_writer.open_file(output_weights_filename);
    weights_file_writer.write_header(weights_header);

    std::vector<char> data_buffer(data_bufsz);
    std::vector<char> weights_buffer(weights_bufsz);

    size_t data_bytes_generated = 0;
    size_t weights_bytes_generated = 0;
    bool data_valid = true;

    char* data_ptr = nullptr;
    char* weights_ptr = nullptr;

    while (data_valid && data_bytes_remaining > 0 && weights_bytes_remaining > 0)
    {
      ssize_t data_bytes_to_generate = std::min(data_bytes_remaining, data_bufsz);
      ssize_t weights_bytes_to_generate = std::min(weights_bytes_remaining, weights_bufsz);

      SPDLOG_DEBUG("Reading {} bytes, remaining={}", data_bytes_to_generate, data_bytes_remaining);

      data_file_writer.write_data(data_ptr, data_bufsz);
      weights_file_writer.write_data(weights_ptr, weights_bufsz);

      data_bytes_generated += data_bytes_to_generate;
      weights_bytes_generated += weights_bytes_to_generate;

      data_bytes_remaining -= data_bytes_to_generate;
      weights_bytes_remaining -= weights_bytes_to_generate;
    }

    data_file_writer.close_file();
    weights_file_writer.close_file();
  }
  catch (std::exception& exc)
  {
    SPDLOG_ERROR("Exception caught: {}", exc.what());
    return_code = 1;
  }

  SPDLOG_DEBUG("return return_code={}", return_code);
  return return_code;
}

void usage()
{
  std::cout << "Usage: ska_pst_generate_file [options] data_file weights_file output_dir" << std::endl;
  std::cout << std::endl;
  std::cout << "  data_file     raw data file containing packed data samples" << std::endl;
  std::cout << "  weights_file  weights file containing scale and weights corresponding to the data file" << std::endl;
  std::cout << "  output_dir    directory to write the unpacked file" << std::endl;
  std::cout << "  -h            print this help text" << std::endl;
  std::cout << "  -o            use O_DIRECT for writing file output" << std::endl;
  std::cout << "  -v            verbose output" << std::endl;
}
