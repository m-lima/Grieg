#ifndef __INF251_SDL__24851580
#define __INF251_SDL__24851580

#include <string>
#include "infdef.hh"

enum struct States {
    none,
    reset,
    translate,
    rotate,
    rotateLight,
    zoom,
    togglePerspective,
    fullScreen
};

struct Update {
    States state = States::none;
    int x = 0;
    int y = 0;
    int oX = 0;
    int oY = 0;
};

namespace Sdl {
  using GlInitProc = void (*)();
  using GlResizeProc = void (*)();
  using GlDisplayProc = void (*)(Update);

  void setGlInit(GlInitProc glInit);

  void setGlResize(GlResizeProc glResize);

  void setGlDisplay(GlDisplayProc glDisplay);

  void mainLoop();

  glm::ivec2 mouseCoords();
  glm::ivec2 screenCoords();
};

#endif //__INF251_SDL__24851580
