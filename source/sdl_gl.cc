#include "SDL.h"
#include "sdl_gl.hh"

namespace {
  std::ostream& operator<<(std::ostream& s, const SDL_version &v)
  {
      return s << format("{:d}.{:d}.{:d}", v.major, v.minor, v.patch);
  }

#ifdef GLAD_DEBUG
  void glad_pre_callback(const char *name, void *, int, ...) {
  }

  void glad_post_callback(const char *name, void *, int, ...) {
      GLenum err;
      err = glad_glGetError();
      if (err != GL_NO_ERROR)
          fatal("GL Error after calling {} with id {}", name, err);
  }
#endif //GLAD_DEBUG
}

struct SdlImpl {
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;

    ~SdlImpl()
    {
        if (context)
            SDL_GL_DeleteContext(context);

        if (window)
            SDL_DestroyWindow(window);
    }
};

Sdl::Sdl():
    mImpl(new SdlImpl)
{
}

Sdl::~Sdl()
{
    /* Empty so that the mImpl destructor isn't generated inline in TUs where
       SdlImpl isn't defined */
}

void Sdl::main_loop()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        fatal("Error initialising SDL: {}", SDL_GetError());

    /* Print SDL version */
    SDL_version compiled, linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    println("SDL2 Header Version: {}", compiled);
    println("SDL2 Library Version: {}", linked);

    /* Specify a OpenGL 4.3 Core profile context */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    mImpl->window = SDL_CreateWindow(mCaption.c_str(),
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     mWidth,
                                     mHeight,
                                     SDL_WINDOW_OPENGL);

    if (!mImpl->window)
        fatal("Error creating SDL Window: {}", SDL_GetError());

    mImpl->context = SDL_GL_CreateContext(mImpl->window);
    if (!mImpl->context)
        fatal("Error creating OpenGL Context: {}", SDL_GetError());

    /* Init GL functions using GLAD */
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    gladLoadGL();

#ifdef GLAD_DEBUG
    glad_set_pre_callback(glad_pre_callback);
    glad_set_post_callback(glad_post_callback);
#endif //GLAD_DEBUG

    /* Print GL Version */
    println("GL Vendor: {}", glGetString(GL_VENDOR));
    println("GL Renderer: {}", glGetString(GL_RENDERER));
    println("GL Version: {}", glGetString(GL_VERSION));
    println("GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

    mGlInit();
    for (;;) {
        /* Event loop */
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                std::exit(1);
                break;

            default:
                break;
            }
        }

        mGlDisplay();
        SDL_GL_SwapWindow(mImpl->window);

        /* Sleep for 10ms */
        SDL_Delay(10);
    }
}
