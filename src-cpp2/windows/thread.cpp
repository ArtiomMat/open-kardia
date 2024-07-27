#include "../thread.hpp"

#include <windows.h>

namespace axe
{
  static DWORD threadproc(thread_t* object)
  {
    object->begin();
    object->_full_end();
    ExitThread(0);
    return 0;
  }

  void thread_t::terminate()
  {
    ending_mutex.lock();
    bool ending = this->ending;
    ending_mutex.unlock();

    if (!ending)
    {
      TerminateThread(id, 0);
      _full_end();
    }
  }
  
  thread_t::~thread_t()
  {
    terminate();
  }

  thread_t::thread_t()
  {
    id = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(threadproc), this, 0, NULL);
  }
}
