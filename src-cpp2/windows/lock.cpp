#include "../lock.hpp"

#include <windows.h>

namespace axe
{
  lock_t::lock_t()
  {
    ptr = new CRITICAL_SECTION;
    InitializeCriticalSection(static_cast<CRITICAL_SECTION*>(ptr));
  }
  lock_t::~lock_t()
  {
    DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(ptr));
    delete ptr;
  }
  void lock_t::lock()
  {
    EnterCriticalSection(static_cast<CRITICAL_SECTION*>(ptr));
  }
  void lock_t::unlock()
  {
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(ptr));
  }
  // lock() but without blocking, if it fails just returns false.
  bool lock_t::try_lock()
  {
    BOOL result = TryEnterCriticalSection(static_cast<CRITICAL_SECTION*>(ptr));
    return result == TRUE;
  }
}