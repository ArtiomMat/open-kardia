#include "mft.hpp"

namespace mft
{
  void thread_t::_full_end()
  {
    ending_mutex.lock();
    ending = true;
    ending_mutex.unlock();
    end();
  }
}