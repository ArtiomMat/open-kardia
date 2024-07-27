#pragma once

namespace axe
{
  // A lock, that can be quite slow.
  struct lock_t
  {
    void* ptr;

    lock_t();
    ~lock_t();

    void lock();
    void unlock();
    // lock() but without blocking, if it fails just returns false.
    bool try_lock();
  };
}