#include "../mt.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <bits/pthreadtypes.h>

typedef struct
{
  void* (*init)(void* args);
  void (*free)(void);
  void* args;
} mt_arg_t;

static void
mt_cleanup_handler(mt_arg_t* arg)
{
  if (arg->free != NULL)
  {
    arg->free();
  }

  free(arg); // Gotta free arg because it was malloc()d
}

static void*
mt_routine(mt_arg_t* arg)
{
  pthread_cleanup_push((void (*)(void*)) mt_cleanup_handler, arg);

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  arg->init(arg->args);

  pthread_cleanup_pop(1);

  return NULL;
}

mt_thr_t
mt_open(mt_thr_t* g, void* (*init)(void* args), void* args, void (*free)(void))
{
  pthread_t t;
  
  mt_arg_t* arg = malloc(sizeof (mt_arg_t));
  arg->init = init;
  arg->args = args;
  arg->free = free;

  if (pthread_create(&t, NULL, (void* (*)(void*)) mt_routine, arg))
  {
    perror("pthread_create");
    return 0;
  }

  *g = (mt_thr_t)t;

  return 1;
}

void
mt_close(mt_thr_t g)
{
  pthread_t t = (pthread_t)g;
  pthread_cancel(t);
}

mt_mtx_t
mt_open_mtx()
{
  pthread_mutex_t* m = malloc(sizeof (*m));
  pthread_mutex_init(m, NULL);
  return m;
}
// XXX: -flto?
void
mt_lock_mtx(mt_mtx_t m)
{
  pthread_mutex_lock(m);
}
void
mt_unlock_mtx(mt_mtx_t m)
{
  pthread_mutex_unlock(m);
}
