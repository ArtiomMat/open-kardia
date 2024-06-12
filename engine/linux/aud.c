#include "../aud.h"

#include <pulse/error.h>
#include <pulse/simple.h>
#include <pulse/pulseaudio.h>

#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024

static int err;

/*
static snd_pcm_t* pcm;
static snd_pcm_hw_params_t* params;
static int16_t* buf;
static int BUF_SIZE;
*/

static pa_simple* client;
static pa_sample_spec ss;
static int error;

static int16_t buf[BUF_SIZE];

int
aud_init()
{
  // Setup the sample spec
  ss.format = PA_SAMPLE_S16LE;
  ss.rate = 16000;
  ss.channels = 1;
  
  // Connect to server
  client = pa_simple_new(NULL, "AUD", PA_STREAM_PLAYBACK, NULL, "AUD_STREAM", &ss, NULL, NULL, &error);

  if (client == NULL)
  {
    fprintf(stderr, "aud_init(): Failed to connect to server, '%s' -PulseAudio\n", pa_strerror(error));
    return 0;
  }

  puts("aud_init(): Simple audio module initialized, no info it's too simple.");
  return 1;
}

void
aud_play(unsigned char freq, unsigned char amp) // Rename parameter to reflect volume level
{
  #ifdef DEBUG
  if (!BUF_SIZE)
  {
    return;
  }
  #endif

  int write_size = BUF_SIZE;
  // How many samples per zig/zag in the sound wave
  int spz = __UINT8_MAX__ - freq + 1;
  int amp16 = amp * (__INT16_MAX__ / __UINT8_MAX__);
  int jps = 2*amp16 / spz; // How much we jump per sample

  buf[0] = 0; // We start at 0

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
  }
  
  if (pa_simple_write(client, buf, write_size, &error) < 0)
  {
    fprintf(stderr, "aud_play(): Failed to write data, '%s' -PulseAudio\n", pa_strerror(error));
  }
}


void
aud_free()
{
  if (client != NULL)
  {
    pa_simple_free(client);
    puts("aud_free(): Audio module freed.");
    return;
  }
  fputs("aud_free(): Nothing to free.\n", stderr);
}

