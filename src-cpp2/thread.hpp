#pragma once

#include "lock.hpp"

namespace axe
{
  struct thread_t
  {
    #ifdef _WIN32
      void* id;
    #elif __linux__
      unsigned long int id;
    #endif
    thread_t();
    ~thread_t();

    bool ending = false;
    lock_t ending_mutex;
    // Premature termination, end() is called.
    void terminate();

    // Called in constructor
    virtual void begin() = 0;
    // Called in destructor, or called when begin returns, or if terminated.
    virtual void end() {}

    // What is actually called.
    void _full_end();
  };
}
