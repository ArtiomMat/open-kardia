#pragma once

#include "com.hpp"
#include "psf.hpp"

namespace dsp
{
  extern bool initialized;

  void initialize(const char* title);
  void shutdown();
}
