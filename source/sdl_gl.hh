#ifndef __INF251_SDL_GL__24851580
#define __INF251_SDL_GL__24851580

#include <string>
#include "infdef.hh"
#include "SDL_keyboard.h"

enum struct States {
    none,
    translate,
    rotate,
	start
};

struct Update {
    States state = States::none;
    unsigned int x = 0;
    unsigned int y = 0;
};

using glinit_type = void (*)(int, int);
using gldisplay_type = void (*)(Update);

struct SdlImpl;

class Sdl {
    glinit_type mGlInit = nullptr;
    gldisplay_type mGlDisplay = nullptr;
    std::string mCaption = "SDL2 Window";
    int mWidth = 800;
    int mHeight = 600;
    std::unique_ptr<SdlImpl> mImpl;

public:
    Sdl();
    ~Sdl();

    void set_glinit(glinit_type glinit_proc)
    {
        mGlInit = glinit_proc;
    }

    void set_gldisplay(gldisplay_type gldisplay_proc)
    {
        mGlDisplay = gldisplay_proc;
    }

    void set_caption(const std::string &caption)
    {
        mCaption = caption;
    }

    void set_resolution(int width, int height)
    {
        mWidth = width;
        mHeight = height;
    }

    void main_loop();
};

#endif //__INF251_SDL_GL__24851580
