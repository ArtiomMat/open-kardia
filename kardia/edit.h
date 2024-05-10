// Editor module, allows for editing heart models.

#pragma once

#include "vid.h"

// Assumes that all modules were initialized, should only be called by the main module.
// Allows for NULL
extern void
edit_init(const char* fp);

extern void
edit_free();

// Piped from Kardia event handler
extern int
edit_on_vid(vid_event_t* e);

extern int
edit_run();
