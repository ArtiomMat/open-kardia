// Multi-fucking threading module

namespace mft
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
    virtual void open() = 0;
    virtual void close() = 0;
  };

  struct mutex_t
  {
    void* ptr;
    mutex_t();
    ~mutex_t();
    void lock();
    void unlock();
    // lock() but without blocking, if it fails just returns false.
    bool try_lock();
  };
}
