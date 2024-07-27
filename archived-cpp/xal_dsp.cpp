#include "xal.hpp"

#include <cstdlib>

extern void main_shutdown_all();

namespace xal
{
  screen_t* scr = nullptr;

  unsigned resolutions_n = 0;
  const resolution_t resolutions[] = {
    resolution_t(320, 240),

    resolution_t(640, 400),
    resolution_t(640, 480),

    resolution_t(800, 500),

    resolution_t(1024, 576),
    resolution_t(1024, 768),
  };

  resolution_t::resolution_t(uint16_t width, uint16_t height) : desc()
  {
    desc.put(width);
    desc.put('x');
    desc.put(height);

    desc.put('(');

    float ratio = 1.0f * width / height;
    int ratio_i = static_cast<int>(ratio*1000);
    
    switch (ratio_i)
    {
      case 1000:
      desc.put("1:1");
      break;
      case 1333:
      desc.put("4:3");
      break;
      case 1600:
      desc.put("16:10");
      break;
      case 1777:
      desc.put("16:9");
      break;

      default:
      throw com::ex_t("Yo, what the hell is this ratio?");
      break;
    }

    desc.put(')');
    desc.optimize();

    puts(desc);

    resolutions_n++;
  }

  screen_t::screen_t(resolution_t& res) : ctx(res.width, res.height)
  {
    cols = res.width / font_f->get_width();
    rows = res.height / font_f->height;
  }

  void dsp_context_t::handler(dsp::window_t::event_t& e)
  {
    switch (e.type)
    {
      case dsp::E_CLOSE:
      total_shutdown(true);
      break;

      case dsp::E_RELEASE:
      
      break;
    }
  }

  void initialize_dsp()
  {
    font_f = new psf::font_t(com::relfp("roman.psf"));

    scr = new screen_t(resolutions[5]);

    // screen = new dsp_char_t[dsp_context->map.width * dsp_context->map.height][1];

    dsp_context->palette[0][0] = 0;
    dsp_context->palette[0][1] = 0;
    dsp_context->palette[0][2] = 0;

    dsp_context->palette[1][0] = 255;
    dsp_context->palette[1][1] = 255;
    dsp_context->palette[1][2] = 255;

    dsp_context->realize_palette();
  }

  void put_screen()
  {

  }
}
