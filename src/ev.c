#include "ev.h"

#include <stdlib.h>

typedef struct
{
  struct hlink_s* prev;
  struct hlink_s* next;
  ev_handler_t handler;
} chan_t;

chan_t channels[EV_C_N] = {0};

void
ev_open(int channel, ev_handler_t handler)
{

}

void
ev_close(int channel, ev_handler_t handler)
{

}

void
ev_raise(int channel, void* e);