// Multi-threading module, simple abstraction over OS-specific system calls.

#include "com.h"

typedef void* mt_thr_t;
// A pointer to an OS defined structure of a mutex
typedef void* mt_mtx_t;
// A pointer to an OS defined structure of a spinlock
typedef void* mt_spn_t;

// init() is called the moment the thread is opened, with its args being args. free(), if not null, is called either when init() returns, or when the gist is closed.
// Returns NULL if fails to open a thread.
extern mt_thr_t
mt_open(mt_thr_t* g, void* (*init)(void* args), void* args, void (*free)(void));

// It's important that AFTER calling this you free everything the gist allocated because the gist wouldn't have the oportunity.
extern void
mt_close(mt_thr_t g);

extern mt_mtx_t
mt_open_mtx();
extern void
mt_lock_mtx(mt_mtx_t m);
extern void
mt_unlock_mtx(mt_mtx_t m);
