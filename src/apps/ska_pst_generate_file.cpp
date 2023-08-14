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

#include "ska/pst/common/utils/SegmentGenerator.h"
#include "ska/pst/common/utils/PacketGeneratorFactory.h"

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

// default duration of output signal
static const double default_duration = 10.0; // seconds

auto main(int argc, char *argv[]) -> int
{
  ska::pst::common::setup_spdlog();

  std::string data_config_filename;
  std::string weights_config_filename;

  std::string signal_generator;
  double duration = default_duration;

  bool use_o_direct = false;

  char verbose = 0;

  opterr = 0;

  int c = 0;

  while ((c = getopt(argc, argv, "d:hos:T:w:v")) != EOF)
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

      case 'd':
        data_config_filename = optarg;
        break;

      case 'w':
        weights_config_filename = optarg;
        break;

      case 's':
        signal_generator = optarg;
        break;

      case 'T':
        duration = atof(optarg);
        break;

      case 'v':
        verbose++;
        break;

      default:
        std::cerr << "ERROR: unrecognised option: -" << static_cast<char>(optopt) << std::endl;
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

  if (data_config_filename.empty())
  {
    SPDLOG_ERROR("ERROR: config filename not specified");
    usage();
    return EXIT_FAILURE;
  }

  if (weights_config_filename.empty())
  {
    SPDLOG_ERROR("ERROR: config filename not specified");
    usage();
    return EXIT_FAILURE;
  }

  if (signal_generator.empty())
  {
    SPDLOG_ERROR("ERROR: signal generator not specified");
    usage();
    return EXIT_FAILURE;
  }

  std::string output_data_dir = "data";
  std::string output_weights_dir = "weights";

  int return_code = 0;

  try
  {
    // load data and weights configurations and set parameters as needed

    ska::pst::common::AsciiHeader data_header;
    ska::pst::common::AsciiHeader weights_header;

    data_header.load_from_file(data_config_filename);
    weights_header.load_from_file(weights_config_filename);

    data_header.set_val("DATA_GENERATOR", signal_generator);

    ska::pst::common::SegmentGenerator generator;
    generator.configure(data_header, weights_header);

    // the SegmentGenerator will initialize some header parameters if necessary
    data_header = generator.get_data_header();
    weights_header = generator.get_weights_header();

    std::string utc_start = data_header.get_val("UTC_START");
    uint32_t file_number = data_header.get_uint32("FILE_NUMBER");
    uint32_t obs_offset = data_header.get_uint32("OBS_OFFSET");

    // create output data and weights folders

    std::filesystem::path output_data_path(output_data_dir);
    std::filesystem::create_directory(output_data_path);

    std::filesystem::path output_weights_path(output_weights_dir);
    std::filesystem::create_directory(output_weights_path);

    // create output filenames

    ska::pst::common::FileWriter data_file_writer(use_o_direct);
    ska::pst::common::FileWriter weights_file_writer(use_o_direct);
    
    std::filesystem::path output_data_filename = output_data_path / data_file_writer.get_filename(utc_start, obs_offset, file_number);
    SPDLOG_DEBUG("ska_pst_generate_file writing data to file {}", output_data_filename.generic_string());

    std::filesystem::path output_weights_filename = output_weights_path / weights_file_writer.get_filename(utc_start, obs_offset, file_number);
    SPDLOG_DEBUG("ska_pst_generate_file writing weights to file {}", output_weights_filename.generic_string());

    // open output files and write headers

    data_file_writer.open_file(output_data_filename);
    data_file_writer.write_header(data_header);

    weights_file_writer.open_file(output_weights_filename);
    weights_file_writer.write_header(weights_header);

    // compute the number of heaps to write to file

    double bytes_per_second = data_header.compute_bytes_per_second();
    auto bytes_per_heap = data_header.get_uint32("RESOLUTION");

    double seconds_per_heap = bytes_per_heap / bytes_per_second;
    auto num_heaps = static_cast<size_t>(floor(duration / seconds_per_heap));

    SPDLOG_DEBUG("ska_pst_generate_file seconds_per_heap={} num_heaps={}", seconds_per_heap, num_heaps);

    unsigned num_heaps_per_loop = 1;
    generator.resize(num_heaps_per_loop);

    for (unsigned iheap=0; iheap < num_heaps; iheap++)
    {
      SPDLOG_INFO("ska_pst_generate_file generating {} of {} heaps", iheap, num_heaps);
      ska::pst::common::SegmentProducer::Segment segment = generator.next_segment();

      ssize_t data_written = data_file_writer.write_data(segment.data.block, segment.data.size);
      if (data_written != segment.data.size)
      {
        SPDLOG_ERROR("ska_pst_generate_file wrote only {} of {} bytes of data", data_written, segment.data.size);
        break;
      }

      ssize_t weights_written = weights_file_writer.write_data(segment.weights.block, segment.weights.size);
      if (weights_written != segment.weights.size)
      {
        SPDLOG_ERROR("ska_pst_generate_file wrote only {} of {} bytes of weight", weights_written, segment.weights.size);
        break;
      }
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
  std::cout << "Usage: ska_pst_generate_file [options]" << std::endl;
  std::cout << std::endl;
  std::cout << "  -d config     name of configuration file for output data" << std::endl;
  std::cout << "  -w config     name of configuration file for output weights" << std::endl;
  std::cout << "  -s signal     name of signal generator (" << ska::pst::common::get_supported_data_generators_list() << ")" << std::endl;
  std::cout << "  -T seconds    duration of simulated signal (default: " << default_duration << ")" << std::endl;
  std::cout << "  -h            print this help text" << std::endl;
  std::cout << "  -o            use O_DIRECT for writing file output" << std::endl;
  std::cout << "  -v            verbose output" << std::endl;
}
