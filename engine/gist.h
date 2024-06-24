// Multi-threading module, simple abstraction over OS-specific system calls.

#include "com.h"

typedef void* gist_t;
// A pointer to an OS defined structure of a mutex
typedef void* gist_mtx_t;

// init is called with the var-args at the end, free(if not NULL) is called once init exits, or once gist_close() is called on the gist returned.
extern gist_t
gist_open(int (*init)(), int (*free)(void), ...);

extern gist_t
gist_close(gist_t g);

extern gist_t
gist_kill();

// If fast_ops is 1 the mutex is intialized to do spinlocks rather than blocks when waiting to aquire it, you should only use it if you are certain that every lock-unlock block will be FAST, atleast sub-second fast.
extern gist_mtx_t
gist_open_mtx(int fast_ops);

extern gist_mtx_t
gist_lock_mtx(gist_mtx_t m);

extern gist_mtx_t
gist_unlock_mtx(gist_mtx_t m);
