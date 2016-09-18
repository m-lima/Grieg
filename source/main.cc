#include <glad/glad.h>
#include "SDL.h"

#include <cstdlib>

#include "infdef.hh"

namespace {
    std::ostream& operator<<(std::ostream& s, const SDL_version &v)
    {
        return s << format("{:d}.{:d}.{:d}", v.major, v.minor, v.patch);
    }

    void print_sdl_info()
    {
        SDL_version compiled, linked;

        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);

        println("SDL2 Header Version: {}", compiled);
        println("SDL2 Library Version: {}", linked);
    }

    void print_gl_info()
    {
        println("GL Vendor: {}", glGetString(GL_VENDOR));
        println("GL Renderer: {}", glGetString(GL_RENDERER));
        println("GL Version: {}", glGetString(GL_VERSION));
        println("GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

    }
}

class SDLPlatform {
    SDL_Window *mWnd = nullptr;
    SDL_GLContext mGL = nullptr;

public:
    SDLPlatform() = default;

    ~SDLPlatform()
    { quit(); }

    void quit()
    {
    }
};

int main()
{
    print("Hello, world!\n");

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    gladLoadGL();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        fmt::print("SDL Error: {}\n", SDL_GetError());
    }

    print_sdl_info();

    SDL_Window *window;
    SDL_GLContext context;

    window = SDL_CreateWindow("INF251", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    // GL Context must exist prior to calling glGetString
    print_gl_info();

    for (;;)
    {
        SDL_Event ev;
        while(SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_QUIT:
                std::exit(0);
                    break;
            }
        }
    }

    return 0;
}