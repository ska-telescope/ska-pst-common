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

#include <utility>

#include "ska/pst/common/utils/SquareWaveGenerator.h"

ska::pst::common::SquareWaveGenerator::SquareWaveGenerator(std::shared_ptr<ska::pst::common::PacketLayout> _layout) :
  ska::pst::common::GaussianNoiseGenerator(std::move(_layout))
{
}

void ska::pst::common::SquareWaveGenerator::configure(const ska::pst::common::AsciiHeader& config)
{
  SPDLOG_DEBUG("ska::pst::common::SquareWaveGenerator::configure");
  ska::pst::common::GaussianNoiseGenerator::configure(config);

  // number of terms that contribute to total intensity: re,im + polA,polB
  const unsigned terms = 4;

  if (config.has("CAL_OFF_INTENSITY"))
  {
    off_stddev = sqrt(config.get_double("CAL_OFF_INTENSITY")/terms);
  }

  if (config.has("CAL_ON_INTENSITY"))
  {
    on_stddev = sqrt(config.get_double("CAL_ON_INTENSITY")/terms);
  }

  if (on_stddev <= off_stddev)
  {
    SPDLOG_WARN("ska::pst::common::SquareWaveGenerator::configure CAL_ON_INTENSITY <= CAL_OFF_INTENSITY");
  }

  if (config.has("CAL_DUTY_CYCLE"))
  {
    duty_cycle = config.get_double("CAL_DUTY_CYCLE");
  }

  if (duty_cycle <= 0.0 || duty_cycle >= 1.0)
  {
    SPDLOG_ERROR("ska::pst::common::SquareWaveGenerator::configure invalid CAL_DUTY_CYCLE={}", duty_cycle);
    throw std::runtime_error("ska::pst::common::SquareWaveGenerator::configure invalid CAL_DUTY_CYCLE");
  }

  if (config.has("CALFREQ"))
  {
    frequency = config.get_double("CALFREQ");
  }

  if (frequency <= 0.0)
  {
    SPDLOG_ERROR("ska::pst::common::SquareWaveGenerator::configure invalid CALFREQ={}", frequency);
    throw std::runtime_error("ska::pst::common::SquareWaveGenerator::configure invalid CALFREQ");
  }

  sampling_interval = config.get_double("TSAMP") / ska::pst::common::microseconds_per_second;
}

void ska::pst::common::SquareWaveGenerator::fill_data(char * buf, uint64_t size)
{
  SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::fill_data buf={} size={}", reinterpret_cast<void*>(buf), size);

  static constexpr uint32_t nbits_per_byte = 8;
  const uint32_t nsamp_per_packet = layout->get_samples_per_packet();
  const uint32_t nchan_per_packet = layout->get_nchan_per_packet();
  const uint32_t nbyte_per_sample = ndim * nbit / nbits_per_byte;
  const uint32_t nbyte_stride = nsamp_per_packet * nbyte_per_sample;;
  const uint32_t narray = nchan_per_packet * npol;
  const uint32_t resolution = narray * nbyte_stride;
  const uint32_t nblocks = size / resolution;

  assert(size % resolution == 0);

  SPDLOG_DEBUG("ska::pst::common::SquareWaveGenerator::fill_data nsamp_per_packet={} nchan_per_packet={} size={} resolution={} nblocks={}",
               nsamp_per_packet, nchan_per_packet, size, resolution, nblocks);

  double phase_per_sample = sampling_interval * frequency;

  SPDLOG_DEBUG("ska::pst::common::SquareWaveGenerator::fill_data sampling_interval={} calfreq={} phase_per_sample={}",
               sampling_interval, frequency, phase_per_sample);

  uint64_t i = 0;
  for (uint32_t iblock=0; iblock<nblocks; iblock++)
  {
    SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::fill_data iblock={}", iblock);

    uint32_t isamp = 0;
    while (isamp < nsamp_per_packet)
    {
      double fractional_phase = fmod( (current_sample+isamp) * phase_per_sample, 1.0);
      uint32_t nsamp = 0;

      SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::fill_data with fractional_phase={}", fractional_phase);

      if (fractional_phase < duty_cycle)
      {
        dat_sequence.set_stddev(on_stddev);
        // number of samples until start of off-pulse region
        nsamp = (duty_cycle - fractional_phase) / phase_per_sample + 1;

        SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::fill_data with {} on-pulse samples", nsamp);
      }
      else
      {
        dat_sequence.set_stddev(off_stddev);
        // number of samples until start of on-pulse region
        nsamp = (1.0 - fractional_phase) / phase_per_sample + 1;

        SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::fill_data with {} off-pulse samples", nsamp);
      }

      if (isamp + nsamp > nsamp_per_packet)
      {
        nsamp = nsamp_per_packet - isamp;
        SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::fill_data truncate to {} samples", nsamp);
      }

      assert (nsamp > 0);

      for (uint32_t ipol=0; ipol<npol; ipol++)
      {
        for (uint32_t ichan=0; ichan<nchan_per_packet; ichan++)
        {
          uint32_t offset = (ipol*nchan_per_packet + ichan)*nbyte_stride + isamp*nbyte_per_sample;
          dat_sequence.generate(buf + offset, nsamp * nbyte_per_sample);
        }
      }

      isamp += nsamp;
    }

    buf += resolution;

    current_channel += nchan_per_packet;
    if (current_channel >= nchan)
    {
      current_channel = 0;
      current_sample += nsamp_per_packet;
    }    
  }
}

auto ska::pst::common::SquareWaveGenerator::test_data(char * buf, uint64_t size) -> bool
{
  SPDLOG_TRACE("ska::pst::common::SquareWaveGenerator::test_data buf={} size={}", reinterpret_cast<void*>(buf), size);

  temp_data.resize(size);
  fill_data(temp_data.data(), size);

  for (uint64_t i=0; i < size; i++)
  {
    if (temp_data[i] != buf[i])
    {
      return false;
    }
  }

  return true;
}
