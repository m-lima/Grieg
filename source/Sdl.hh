#ifndef __INF251_SDL__24851580
#define __INF251_SDL__24851580

#include <string>
#include "infdef.hh"

enum struct States {
    none,
	start,
    translate,
    rotate,
	zoom,
	fov
};

struct Update {
    States state = States::none;
    int x = 0;
    int y = 0;
};

namespace Sdl {
	using GlInitProc = void (*)();
	using GlDisplayProc = void (*)(Update);

    void setGlInit(GlInitProc glInit);

    void setGlDisplay(GlDisplayProc glDisplay);

    void mainLoop();

    glm::ivec2 mouseCoords();
    glm::ivec2 screenCoords();
};

#endif //__INF251_SDL__24851580
