#include "wav.hpp"
#include "tmr.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace wav
{
  bool initialized = false;

  char volume = 50;

  unsigned char* samples;
  int write_samples_n;

  void initialize()
  {
    // If initialized and a reasonable time(more than 10ms)
    if (tmr::initialized && tmr::target_tick_time > 10)
    {
      initialize(tmr::target_tick_time);
    }
    else
    {
      initialize(30);
    }
  }

  // Scales x from old_bits bits, to new_bits bits. For instance it's used to scale 8 bit number to 16 bit, 1 becomes 16.
  static int scale_bits(int x, int old_bits, int new_bits)
  {
    if (new_bits > old_bits)
    {
      return x << (new_bits - old_bits);
    }
    else
    {
      return x >> (old_bits - new_bits);
    }
  }

  file_t::file_t(const char* fp)
  {
    FILE* f = fopen(fp, "rb");
    if (!f) {
      throw com::open_ex_t("Opening file.");
    }

    // Test the RIFF
    char riff[4];
    fread(riff, 4, 1, f);
    if (memcmp("RIFF", riff, 4)) {
      fclose(f);
      throw com::open_ex_t("RIFF header not found, not a .WAV file.");
    }

    // NOTE: We already read the "RIFF" part.
    // Skipping: Size in bytes + Wave marker + Fmt marker + Fmt header length
    fseek(f, 4+4+4+4, SEEK_CUR);

    uint16_t format;
    fread(&format, 2, 1, f);
    format = com::lil16(format);
    // Not PCM?
    if (format != 1) {
      fclose(f);
      throw com::open_ex_t("Only PCM format supported.");
    }
    
    uint16_t channels_n;
    fread(&channels_n, 2, 1, f);
    channels_n = com::lil16(channels_n);

    if (channels_n != 1 && channels_n != 2)
    {
      fclose(f);
      throw com::open_ex_t("Only 1/2 channel audio is supported.");
    }

    uint32_t sample_rate;
    fread(&sample_rate, 4, 1, f);
    sample_rate = com::lil32(sample_rate);

    // Skipping: byte rate + block alignment
    fseek(f, 4+2, SEEK_CUR);

    uint16_t bits_per_sample;
    fread(&bits_per_sample, 2, 1, f);
    bits_per_sample = com::lil16(bits_per_sample);

    // Skipping: Data marker or List chunk if present
    fread(riff, 4, 1, f);
    if (!memcmp("LIST", riff, 4)) {
      uint32_t i;
      fread(&i, 4, 1, f);
      fseek(f, i+4, SEEK_CUR);
    }

    uint32_t datasize;
    fread(&datasize, 4, 1, f);
    datasize = com::lil32(datasize);

    samples_n = datasize / (channels_n * (bits_per_sample/8));

    samples = new char[datasize];
    if (samples == nullptr)
    {
      fclose(f);
      throw com::memory_ex_t("Allocating samples.");
    }

    fread(samples, datasize, 1, f);
    fclose(f);

    // First, do we need to convert?
    if (bits_per_sample == BITS_PER_SAMPLE && channels_n == CHANNELS_N && sample_rate == SAMPLE_RATE)
    {
      return;
    }

    // If we do, then well shit time to convert
    char* old_samples = samples;
    int old_samples_n = samples_n;

    // How many samples in the old audio there are per single sample in the converted audio.
    float rate_ratio = 1.0f * sample_rate / SAMPLE_RATE;

    samples_n = samples_n / rate_ratio; // / because inverse
    samples = new char[samples_n * CHANNELS_N * (BITS_PER_SAMPLE/8)];
    if (samples == nullptr)
    {
      delete [] old_samples;
      throw com::memory_ex_t("Allocating samples.");
    }

    for (unsigned sample_i = 0; sample_i < samples_n; sample_i++)
    {
      unsigned old_sample_i = sample_i * rate_ratio;

      // TODO
    }

    delete [] old_samples;
  }

  file_t::~file_t()
  {
    delete [] samples;
  }

  source_t::source_t(file_t& f) : audio(f)
  {
    
  }

  void source_t::play()
  {
    uint32_t j = sample_i * CHANNELS_N;

    for (uint32_t i = 0; i < write_samples_n * CHANNELS_N; i++, j++)
    {
      if (j >= this->audio.samples_n * CHANNELS_N)
      {
        if (loop)
        {
          j = 0;
        }
        else
        {
          break;
        }
      }

      switch (BITS_PER_SAMPLE)
      {
        case 8:
        {
          int8_t& amp8 = reinterpret_cast<int8_t*>(this->audio.samples)[j];
          reinterpret_cast<int8_t*>(samples)[i] += (int16_t)amp8 / 2;
        }
        break;

        case 16:
        {
          int16_t& amp16 = reinterpret_cast<int16_t*>(this->audio.samples)[j];
          reinterpret_cast<int16_t*>(samples)[i] += (int32_t)amp16 / 2;
        }
        break;

        case 32:
        {
          int32_t& amp32 = reinterpret_cast<int32_t*>(this->audio.samples)[j];
          reinterpret_cast<int32_t*>(samples)[i] += (int64_t)amp32 / 2;
        }
        break;
      }
    }

    sample_i = j / CHANNELS_N;
  }
}