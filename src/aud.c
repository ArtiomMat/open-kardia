#include "aud.h"
#include "clk.h"

#include <alsa/asoundlib.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

static int err;

static snd_pcm_t* pcm;
static snd_pcm_hw_params_t* params;
static int16_t* buf;
static int buf_size;

static unsigned int sample_rate;

int
aud_init(unsigned int _sample_rate)
{
  sample_rate = _sample_rate;  
  // Open device into pcm
  if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
  {
    printf("aud_init(): Opening PCM device failed: %s\n", snd_strerror(err));
    return 0;
  }

  // Allocate params
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(pcm, params);

  // Setup params
  snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED); // read/write access
  snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_S16_LE); // 8 bit signed samples
  snd_pcm_hw_params_set_channels(pcm, params, 1); // Just one channel
  snd_pcm_hw_params_set_rate_near(pcm, params, &sample_rate, 0); // Sample  rate

  // Send params to the pcm device
  if ((err = snd_pcm_hw_params(pcm, params)) < 0)
  {
    printf("aud_init(): Sending parameters to PCM device failed: %s\n", snd_strerror(err));
    return 0;
  }

  // Calculate and allocate buf size using "period size", no clue, but that's what it do
  {
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_get_period_size(params, &frames, 0);
    buf_size = frames * sizeof (*buf);
  }
  buf = malloc(sizeof (*buf) * buf_size);


  printf("aud_init(): Audio module initialized, %d bytes allocated for %dHz.\n", sizeof (*buf) * buf_size, sample_rate);

  return 1;
}

void
aud_play(unsigned char freq, unsigned char amp) // Rename parameter to reflect volume level
{
  int write_size = sample_rate/15;
  // How many samples per zig/zag in the sound wave
  int spz = __UINT8_MAX__ - freq + 1;
  int amp16 = amp * (__INT16_MAX__ / __UINT8_MAX__);
  int jps = 2*amp16 / spz; // How much we jump per sample

  buf[0] = amp16; // We start at the top
  
  // Set all samples to the desired level (0-127 for 8-bit signed)
  for (int i = 1, sign = -1; i < write_size; i++)
  {
    int sample = sign * jps + buf[i-1];
    if (sample * sign >= amp16)
    {
      sample = amp16 * sign;
      sign = -sign;
    }
    
    buf[i] = sample;
  printf("%f\n", 1.0f * buf[i] / __INT16_MAX__);
  }

  snd_pcm_prepare(pcm);

  if ((err = snd_pcm_writei(pcm, buf, write_size)) != write_size)
  {
    printf("aud_play(): Didn't write everything...\n", snd_strerror(err));
  }
}


void
aud_free()
{
  snd_pcm_close(pcm);
  free(buf);
  puts("aud_free(): Audio module freed.");
}
