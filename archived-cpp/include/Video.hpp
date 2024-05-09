#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include "Events.hpp"
#include "Noter.hpp"

namespace System
{
  enum
  {
    KEY_ESC=-64,
    KEY_ENTER,
    KEY_SPACE,
    KEY_BS,
    KEY_TAB,
    KEY_RALT,
    KEY_CTRL,
    KEY_LALT,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_CAPSLOCK,
    KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,
    KEY_NUMLOCK,KEY_SCROLLLOCK,

    KEY_LMOUSE, // Left mouse
    KEY_MMOUSE, // Middle mouse
    KEY_RMOUSE, // Right mouse
  };

  // A structure that acts like a bitmap, provided by Video for portable bitmap creation :)
  // Notice that there is no internal pixel data or anything, it aims to be as minimal as possible because for instance Video uses it and the pixels are stored somewhere else!
  class BitmapLike
  {
    public:
    int size[2];

    virtual ~BitmapLike() = default;

    virtual unsigned char getPixel(int i) = 0;
    virtual void setPixel(int i, unsigned char c) = 0;
  };

  struct VideoEvent
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

  class VideoListener : public EventHandler<VideoEvent>
  {
    public:
    enum Type
    {
      _EV_NULL,
      EV_CLOSE, // Close event, user wants to close the application
      EV_HIDE, // When the video module is hidden and cannot be seen by the user
      EV_SHOW, // When the video module is shown and seen by the user(sends it on vid_init() too)

      EV_PRESS, // Press key/button
      EV_RELEASE, // Release key/button
      EV_MOVE, // Move mouse
    };
  };

  // Contains various system data
  struct SystemVideo;

  class Video : public BitmapLike
  {
    private:
    SystemVideo* sys;
    // Refresh rate of screen, should not be read before initialization of a Video object
    static int screenRate;

    void free();

    Noter noter;
    EventReporter<VideoEvent> reporter;

    public:
    // Contains RGB pairs for all the colors that the video instance can use
    unsigned char colors[256][3];

    Video(int width, int height);
    Video(int width, int height, VideoListener& listener);
    ~Video();

    unsigned char getPixel(int i) override;
    void setPixel(int i, unsigned char c) override;

    void setTitle(const char* title);
    // Wipe the screen with a color
    void wipe(unsigned char color);
    // Draw all the changes on the screen, also announces all queued events
    void endFrame();
  };
}

#endif // VIDEO_HPP_INCLUDED
