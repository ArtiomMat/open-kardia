#include "dsp.hpp"

#include <cstdlib>

namespace dsp
{
  bool initialized = false;

  void ctx_t::handler(event_t& e)
  {
    if (e.type == E_CLOSE)
    {
      exit(0);
    }
  }

}
