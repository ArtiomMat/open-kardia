// NOTE: UNUSED!

// Event manager, because I have too many modules that need to interact through events, yep.

#pragma once

#include <stddef.h>

// PLEASE ADD THE CHANNELS AS YOU WANT TO HERE
// MAKE SURE TO RECOMPILE THE ENTIRE PROJECT AFTER MODIFICATION!
enum
{
  EV_C_VID,
  EV_C_GUI,

  EV_C_N, // Don't touch
};

typedef void (*ev_handler_t)(void* e);

// If you have so many modules that you need a dynamic number of channels email me at xxx_idclol_xxx@gmail.com
extern void
ev_init(int channels_n);

extern void
ev_free();

// Open a connection of a handler to the channel.
extern void
ev_open(int channel, ev_handler_t handler);

// Close a handler connection with the channel.
extern void
ev_close(int channel, ev_handler_t handler);

// Raise the event to all the connected handlers
// The calls to the handlers happen in the reverse order, last handler to be added to the channel is called first.
extern void
ev_raise(int channel, void* e);
