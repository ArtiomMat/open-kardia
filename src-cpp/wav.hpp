#pragma once

#include "com.hpp"

namespace wav
{
  struct file_t
  {
    void* samples; // The frames flattened into a raw sample array.
    uint32_t samples_n; // Samples per channel
    uint16_t rate; // The rate of samples PER CHANNEL, not total samples. a better name is framerate, but, terminology I guess.
    uint8_t channels_n; // How many channels per frame
    uint8_t bps;

    // Throws appropriate file_ex_t(specific ones).
    file_t(const char* fp);
    ~file_t();
  };

  struct source_t
  {
    file_t& audio;
    uint32_t sample_i = 0; // Index of current sample.
    char volume = 100;
    bool loop = false ; // Loop the source when it reaches end.
    bool playing = false; // If playing right now.
  
    source_t(file_t& f);
    ~source_t() = default;

    // Renders the audio, but must call wav::run() too.
    void play();
    void rewind() { sample_i = 0; }
  };

  // Scale of 0 to 100, this is the master volume, by default around 50 to not fuck up the ears.
  extern char volume;

  extern bool initialized;

  // Throws system_ex_t if fails.
  void initialize();
  
  // Plays all audios to the speaker, this must be called at the appropriate rate each frame.
  void run();


  void shutdown();
}