#include "Video.hpp"
#include "Errors.hpp"

#include <iostream>

int main()
{
  long long x = 0xFFFFFFFFFFFFFFFF;
  int y = static_cast<int>(x);
  try
  {
    System::Video video(400, 500);

    while (true)
    {
      video.endFrame();
    }
  }
  catch (Error& e)
  {
    std::cout << "Error: " << e.str << '\n';
  }
  return 0;
}
