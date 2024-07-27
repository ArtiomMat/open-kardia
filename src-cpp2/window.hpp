#pragma once

#include "image.hpp"

namespace axe
{
  struct system_data_t;

  // It is highly recommended to inherit this class and override handler to something that suits you!
  struct window_t
  {
    enum event_type_t
    {
      _E_NULL,
      E_CLOSE, // Close event, user wants to close the application
      E_HIDE, // When the video module is hidden and cannot be seen by the user
      E_SHOW, // When the video module is shown and seen by the user(sends it on context_t() too)

      E_PRESS, // Press key/button
      E_RELEASE, // Release key/button
      E_MOVE, // Move mouse
    };

    struct event_t
    {
      int type;
      union
      {
        struct
        {
          int code;
        } press, release;
        struct
        {
          int x, y;
        } move;
      };
    };

    system_data_t* sys = nullptr; // Means that it's uninitialized
    image_t image;
    char palette[256][3];
    char pixel_size;

    virtual void handler(event_t& e);

    // Throws com::system_ex_t if fails.
    window_t(short w, short h);
    virtual ~window_t();

    // handler will be called if events are present, so make sure to set it up!
    void run();
    // Refreshes the actual context on the system level, the window buffer, using image.
    void refresh();
    // Realizes the palette, many systems require copying of the palette to an internal buffer.
    void realize_palette();
  };
}