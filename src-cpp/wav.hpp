#pragma once

#include "com.hpp"

namespace wav
{
  struct file_t
  {
    char* samples; // The frames flattened into a raw sample array.
    uint32_t samples_n; // Samples per channel

    // Only can open WAV files!
    // Throws appropriate file_ex_t(specific ones).
    file_t(const char* fp);
    ~file_t();
  };

  struct source_t
  {
    file_t& audio;
    uint64_t sample_i = 0; // Index of current sample.
    char volume = 100;
    bool loop = false ; // Loop the source when it reaches end.
    bool playing = false; // If playing right now.
  
    source_t(file_t& f);
    ~source_t() = default;

    // Renders the audio, but must call wav::run() too. If reaches end and loop is false then we just stop playing automatically.
    void play();
    void rewind() { sample_i = 0; }
  };

  // Scale of 0 to 100, this is the master volume, by default around 50 to not fuck up the ears.
  extern char volume;

  extern bool initialized;

  extern unsigned char* samples;
  extern int write_samples_n;
  
  // I am unsure yet if this should be changed, for now it shouldn't.
  static constexpr int BITS_PER_SAMPLE = 16;
  // MUST BE either 1 or 2, the inner working of wav depend on this fact.
  static constexpr int CHANNELS_N = 2;
  static constexpr int SAMPLE_RATE = 48'000;

  // buffer_duration is the size of the audio buffer in milliseconds, for the engine you could use tmr::target_tick_time, but only if your program doesn't run slower than it, if it does, set it to the average time for a frame, if unsure just call the other empty overload.
  // Throws system_ex_t if fails.
  void initialize(unsigned buffer_duration);
  void initialize();
  
  // Begins by retreiving the buffer, after this you should call all the play() on the source_t's you created.
  void begin();
  // Ends the buffer and renders the buffer to the speakers to play!
  void end();

  void shutdown();
}