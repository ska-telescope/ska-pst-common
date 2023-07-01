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

#include <cmath>
#include <algorithm>
#include <spdlog/spdlog.h>

#include "ska/pst/common/utils/SineWaveGenerator.h"

ska::pst::common::SineWaveGenerator::SineWaveGenerator(std::shared_ptr<ska::pst::common::DataLayout> _layout) :
  DataGenerator(_layout), wts_sequence('\xff'), scl_sequence(1.0)
{
}

void ska::pst::common::SineWaveGenerator::configure(const ska::pst::common::AsciiHeader& config)
{
  SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::configure");

  ska::pst::common::DataGenerator::configure(config);

  // determine the amplitude when nbit == ?
  //  8 -> -127 to  127
  // 12 -> -2047 to 2048
  // 16 -> -32767 to 32767
  amplitude = (pow(2, nbit) / 2) - 1;

  SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::configure amplitude={}", amplitude);

  // by default the sinusoid will appear in channel 0
  sinusoid_channel = 0;

  // determine the frequency channel into which the sinusoid should be injected
  if (config.has("SINUSOID_FREQ"))
  {
    double sinusoid_freq = config.get_double("SINUSOID_FREQ"); // MHz
    double freq = config.get_double("FREQ"); // MHz
    double bw = config.get_double("BW"); // MHz
    double chan_bw = fabs(bw / double(nchan));
    double sfreq = freq - (bw / 2);
    SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::configure freq={} bw={} nchan={} chan_bw={}", freq, bw, nchan, chan_bw);
    for (unsigned ichan=0; ichan<nchan; ichan++)
    {
      double from = sfreq + (ichan * chan_bw);
      double to = sfreq + ((ichan + 1) * chan_bw);
      if ((sinusoid_freq >= from) && (sinusoid_freq < to))
      {
        sinusoid_channel = ichan;
      }
    }
  }

  wts_block_offset = layout->get_packet_scales_size();
  wts_block_size = layout->get_packet_weights_size();
  scl_block_offset = 0;
  scl_block_size = layout->get_packet_scales_size();
  block_stride = layout->get_packet_weights_size() + layout->get_packet_scales_size();

  SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::configure sinusoid_channel={}", sinusoid_channel);
}

void ska::pst::common::SineWaveGenerator::fill_data(char * buf, uint64_t size)
{
  SPDLOG_TRACE("ska::pst::common::SineWaveGenerator::fill_data nbit={} buf={} size={}", nbit, reinterpret_cast<void *>(buf), size);
  if (nbit == 8)
  {
    fill_complex_data<int8_t>(buf, size);
  }
  else if (nbit == 16)
  {
    SPDLOG_TRACE("ska::pst::common::SineWaveGenerator::fill_data fill_complex_data<int16_t>(buf, size)");
    fill_complex_data<int16_t>(buf, size);
  }
}

void ska::pst::common::SineWaveGenerator::fill_weights(char * buf, uint64_t size)
{
  wts_sequence.generate_block(buf, size, wts_block_offset, wts_block_size, block_stride);
}

void ska::pst::common::SineWaveGenerator::fill_scales(char * buf, uint64_t size)
{
  scl_sequence.generate_block(buf, size, scl_block_offset, scl_block_size, block_stride);
}

auto ska::pst::common::SineWaveGenerator::test_data(char * buf, uint64_t size) -> bool
{
  SPDLOG_DEBUG("ska::pst::common::SineWaveGenerator::test_data nbit={} buf={} size={}", nbit, reinterpret_cast<void *>(buf), size);
  if (nbit == 8)
  {
    SPDLOG_TRACE("ska::pst::common::SineWaveGenerator::test_data test_complex_data<int8_t>(buf, size)");
    return test_complex_data<int8_t>(buf, size);
  }
  else if (nbit == 16)
  {
    SPDLOG_TRACE("ska::pst::common::SineWaveGenerator::test_data test_complex_data<int16_t>(buf, size)");
    return test_complex_data<int16_t>(buf, size);
  }
  else
  {
    return false;
  }
}

auto ska::pst::common::SineWaveGenerator::test_weights(char * buf, uint64_t size) -> bool
{
  return wts_sequence.validate_block(buf, size, wts_block_offset, wts_block_size, block_stride);
}

auto ska::pst::common::SineWaveGenerator::test_scales(char * buf, uint64_t size) -> bool
{
  return scl_sequence.validate_block(buf, size, scl_block_offset, scl_block_size, block_stride);
}

void ska::pst::common::SineWaveGenerator::reset()
{
  current_sample = 0;
  current_channel = 0;
  wts_sequence.reset();
  scl_sequence.reset();
}
