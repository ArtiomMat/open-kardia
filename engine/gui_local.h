#pragma once

#include "gui.h"

#define TICKBOX_SIZE 10

#define BORDER_THICKNESS (GUI_BORDER_WH>>1)

#define BORDER_RIGHT(WND) (WND.pos[0] + WND.size[0] - 1)
#define BORDER_LEFT(WND) (WND.pos[0])
#define BORDER_TOP(WND) (WND.pos[1])
#define BORDER_BOTTOM(WND) (WND.pos[1] + WND.size[1] - 1)

#define TITLE_RIGHT(WND) (BORDER_RIGHT(WND) - BORDER_THICKNESS)
#define TITLE_LEFT(WND) (BORDER_LEFT(WND) + BORDER_THICKNESS)
#define TITLE_TOP(WND) (BORDER_TOP(WND) + BORDER_THICKNESS)
#define TITLE_BOTTOM(WND) (TITLE_TOP(WND) + gui_title_h - 1)

// All that is used because X is inside of the title so it inherits some stuff
#define X_LEFT(WND) (TITLE_RIGHT(WND) - X_WIDTH)
#define X_BOTTOM(WND) (TITLE_BOTTOM(WND) - 2)
#define X_WIDTH 12

#define CONTENT_RIGHT(WND) (TITLE_RIGHT(WND)-1)
#define CONTENT_LEFT(WND) (TITLE_LEFT(WND)+1)
#define CONTENT_TOP(WND) (TITLE_BOTTOM(WND)+1)
#define CONTENT_BOTTOM(WND) (BORDER_BOTTOM(WND) - BORDER_THICKNESS - 1)

// A bitmap that is always supposed to be vid_px.s[0]*vid_px.s[1].
// An index is in it is an index of a pixel on the screen, and every index references the thing at that pixel
extern gui_thing_t** gui_thing_refs;
