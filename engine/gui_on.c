#include "gui.h"
#include "gui_local.h"
#include "com.h"

#include <stdio.h>

// Part of GUI for input handling

int gui_mouse_pos[2] = {0};
static int mouse_state; // 1 for pressed, 0 for not!

static void
send_event(gui_event_t* e)
{
  if (gui_on != NULL && !gui_on(e))
  {
    puts("FUCK!");
  }
}

static void
save_mouse_rel()
{
  gui_u_t mouse_x = min(max(gui_mouse_pos[0], 0), vid_size[0]-1);
  gui_u_t mouse_y = min(max(gui_mouse_pos[1], 0), vid_size[1]-1);

  gui_window.window.mouse_rel[0] = mouse_x - gui_window.pos[0];
  gui_window.window.mouse_rel[1] = mouse_y - gui_window.pos[1];
}

// i is the dimention of the size/pos vector
// Can also be used to resize towards the bottom of the screen, i is for that
static void
resize_right(int i, gui_u_t max_r)
{
  int mouse_delta = gui_mouse_pos[i] - (gui_window.window.mouse_rel[i] + gui_window.pos[i]);

  gui_window.size[i] = gui_window.window.size_0[i] + mouse_delta;
  gui_window.size[i] = min(max(gui_window.size[i], gui_window.min_size[i]), max_r + 1 - gui_window.pos[i]);
}
// i is the dimention of the size/pos vector
// Can also be used to resize to the top of the screen
static void
resize_left(int i, gui_u_t min_l)
{
  int mouse_delta = gui_mouse_pos[i] - (gui_window.window.mouse_rel[i] + gui_window.pos[i]);

  gui_window.pos[i] += mouse_delta;

  if (gui_window.pos[i] < min_l)
  {
    mouse_delta -= gui_window.pos[i];
    gui_window.pos[i] = 0;
  }

  gui_window.size[i] = gui_window.window.size_0[i] - mouse_delta;

  if (gui_window.size[i] < gui_window.min_size[i])
  {
    gui_window.pos[i] -= gui_window.min_size[i] - gui_window.size[i];
    gui_window.size[i] = gui_window.min_size[i];
  }

  // So that in the next call to resize_left(), the size[i] does not return to be size_0(old). Since the X keeps updating, we need to shift the width with it, so that mouse_delta keeps being current. I know this is not really a good explanation, but just run this on a fucking paper, just shift the window do all the operations without this line, you will understand what I mean.
  gui_window.window.size_0[i] = gui_window.size[i];
}

/**
 * Test if X_TEST and Y_TEST are within a rectangle.
*/
static inline int
in_rect(gui_u_t x_test, gui_u_t y_test, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
  return x_test <= right && x_test >= left && y_test <= bottom && y_test >= top;
}

static void
mouse_move_window(gui_u_t l, gui_u_t t, gui_u_t r, gui_u_t b)
{
  // Move the window if necessary and other logic to keep track of movement
  if (gui_window.window.flags & GUI_WND_RELOCATING)
  {
    gui_event_t e;
    gui_window.pos[0] = e.relocate.delta[0] = gui_mouse_pos[0]-gui_window.window.mouse_rel[0];
    gui_window.pos[1] = e.relocate.delta[1] = gui_mouse_pos[1]-gui_window.window.mouse_rel[1];

    gui_window.pos[0] = e.relocate.normalized[0] = min(max(gui_window.pos[0], l), r+1-gui_window.size[0]);
    gui_window.pos[1] = e.relocate.normalized[1] = min(max(gui_window.pos[1], l), b+1-gui_window.size[1]);
    send_event(&e);
  }
  // Resize the window and keep track of resizing too
  else if (gui_window.window.flags & GUI_WND_RESIZING)
  {
    int flag = gui_window.window.flags & GUI_WND_RESIZING;

    if (flag & GUI_WND_RESIZING_R)
    {
      resize_right(0, r);
    }
    else if (flag & GUI_WND_RESIZING_L)
    {
      resize_left(0, l);
    }

    if (flag & GUI_WND_RESIZING_B)
    {
      resize_right(1, b);
    }
    else if (flag & GUI_WND_RESIZING_T)
    {
      resize_left(1, t);
    }
  }
}

// gui_e is a pointer to the gui event that would be sent, if its ->type is untouched nothing will be sent.
static inline void
window_on_vid(gui_thing_t* gui_window, vid_event_t* e, gui_event_t* gui_e)
{
  switch (e->type)
  {
    case VID_E_MOVE:
    // TODO: MAke this the actual size the window is allowed to occupy
    mouse_move_window(0,0, vid_size[0], vid_size[1]);
    break;

    case VID_E_PRESS:
    if (e->press.code == KEY_LMOUSE || e->press.code == KEY_RMOUSE || e->press.code == KEY_MMOUSE)
    {
      mouse_state = 1;

      // Inside of the title bar
      if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], TITLE_LEFT((*gui_window)), TITLE_TOP((*gui_window)), TITLE_RIGHT((*gui_window)), TITLE_BOTTOM((*gui_window))))
      {
        if (gui_mouse_pos[0] >= X_LEFT((*gui_window)) && gui_mouse_pos[1] <= X_BOTTOM((*gui_window)))
        {
          gui_set_flag(GUI_WND_HIDE, 1);
          gui_set_flag(GUI_WND_UNFOCUSED, 1); // Also we unfocus so the user can interact again

          (*gui_e).type = GUI_E_HIDE;
          send_event(gui_e);
          break;
        }

        if (e->press.code == KEY_LMOUSE)
        {
          gui_set_flag(GUI_WND_RELOCATING, 1);
          save_mouse_rel();
        }
        // else if (e->press.code == KEY_MMOUSE)
        // {
        //   gui_set_flag(GUI_WND_HIDE, 1);
        //   gui_set_flag(GUI_WND_UNFOCUSED, 1); // Also we unfocus so the user can interact again

        //   (*gui_e).type = GUI_E_HIDE;
        //   send_event(&(*gui_e));
        //   break;
        // }
        else if (e->press.code == KEY_RMOUSE)
        {
          gui_toggle_flag(GUI_WND_XRAY); // Toggle!
        }

        // We need to automatically focus back the window regardless, we don't reach this is middle clicked
        gui_window->window.flags &= ~GUI_WND_UNFOCUSED;

        gui_set_flag(GUI_WND_UNFOCUSED, 0);
        (*gui_e).type = GUI_E_UNFOCUS;
        break;
      }
      // Inside of the content zone
      else if (!(gui_window->window.flags & GUI_WND_XRAY) && in_rect(gui_mouse_pos[0], gui_mouse_pos[1], CONTENT_LEFT((*gui_window)), CONTENT_TOP((*gui_window)), CONTENT_RIGHT((*gui_window)), CONTENT_BOTTOM((*gui_window))))
      {
        gui_set_flag(GUI_WND_UNFOCUSED, 0);
        (*gui_e).type = GUI_E_HIDE;
        break;
      }
      // We are 100% either in border or outside the window alltogether
      else
      {
        int flags_tmp = gui_window->window.flags; // Save the flag for future

        // Right and left border
        if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], CONTENT_RIGHT((*gui_window))+1 - GUI_RESIZE_BLEED, BORDER_TOP((*gui_window)), BORDER_RIGHT((*gui_window)), BORDER_BOTTOM((*gui_window))))
        {
          gui_set_flag(GUI_WND_RESIZING_R, 1);
        }
        else if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], BORDER_LEFT((*gui_window)), BORDER_TOP((*gui_window)), CONTENT_LEFT((*gui_window))-1 + GUI_RESIZE_BLEED, BORDER_BOTTOM((*gui_window))))
        {
          gui_set_flag(GUI_WND_RESIZING_L, 1);
        }

        // Top and bottom border, disconnected to combine the two in the corners
        if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], BORDER_LEFT((*gui_window)), BORDER_TOP((*gui_window)), BORDER_RIGHT((*gui_window)), TITLE_TOP((*gui_window))-1 + GUI_RESIZE_BLEED))
        {
          gui_set_flag(GUI_WND_RESIZING_T, 1);
        }
        else if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], BORDER_LEFT((*gui_window)), CONTENT_BOTTOM((*gui_window))+1 - GUI_RESIZE_BLEED, BORDER_RIGHT((*gui_window)), BORDER_BOTTOM((*gui_window))))
        {
          gui_set_flag(GUI_WND_RESIZING_B, 1);
        }

        // NOTE: Introduces thread unsafety because we do comparison if flags changed assuming they can't outside this scope.
        if (gui_window->window.flags != flags_tmp)
        {
          gui_window->window.size_0[0] = gui_window->size[0];
          gui_window->window.size_0[1] = gui_window->size[1];

          save_mouse_rel();
          gui_set_flag(GUI_WND_UNFOCUSED, 0);
          (*gui_e).type = GUI_E_FOCUS;
          break;
        }
        // 100% outside the window, if not already set we set and put the event for eaten
        // We should only send the event if it's the first time, sending the event otherwise is both unnecessary and causes bugs(cannot interact outside GUI)
        else if (!(gui_window->window.flags & GUI_WND_UNFOCUSED))
        {
          gui_set_flag(GUI_WND_UNFOCUSED, 1);
          (*gui_e).type = GUI_E_UNFOCUS;
          break;
        }
        // Otherwise it's not eaten ofc
      }
    }
    break;

    case VID_E_RELEASE:
    if (e->press.code == KEY_LMOUSE || e->press.code == KEY_RMOUSE || e->press.code == KEY_MMOUSE)
    {
      gui_set_flag(GUI_WND_RELOCATING, 0);
      gui_set_flag(GUI_WND_RESIZING, 0);
    }
    break;
  }
}

int
gui_on_vid(vid_event_t* e)
{
  switch (e->type)
  {
    case VID_E_MOVE:
    // We also limit it to avoid any possible segfault
    gui_mouse_pos[0] = max(min(e->move.x, vid_size[0]), 0);
    gui_mouse_pos[1] = max(min(e->move.y, vid_size[1]), 0);
    break;
  }

  gui_event_t gui_e = {.type = _GUI_E_NULL};

  // Now pass on to the specific types
  gui_thing_t* t = gui_thing_refs[gui_mouse_pos[0] + gui_mouse_pos[1] * vid_size[0]];
  if (t != NULL)
  {
    printf("%llu\n", (unsigned long long)t%8);
    if (!(t->flags & GUI_T_HIDE))
    {
      return 0;
    }

    switch (t->type)
    {
      case GUI_T_WINDOW:
      puts("LOL");
      window_on_vid(t, e, &gui_e);
      break;
    }

  }

  // Decide if we ate or not!
  if (gui_e.type == _GUI_E_NULL)
  {
    return 0;
  }
  send_event(&gui_e);
  return 1;
}

