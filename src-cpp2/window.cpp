#include "window.hpp"

#include <cstdlib>

namespace axe
{
  void window_t::handler(event_t& e)
  {
    if (e.type == E_CLOSE)
    {
      exit(0);
    }
  }
}