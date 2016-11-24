#include <memory>
#include <cstdlib>
#include "SDL.h"
#include "SDL_image.h"
#include "Sdl.hh"

extern GLuint gUseProgram;
extern bool gSun;
extern bool gSpotlight;
extern bool gMoveLights;
extern int gNumLights;
extern float gAmbient;
extern bool gRotateModel;
extern int gShaderMode;
extern int gModel;
extern bool gWaterized;

namespace {
  std::ostream& operator<<(std::ostream& s, const SDL_version &v) {
    return s << format("{:d}.{:d}.{:d}", v.major, v.minor, v.patch);
  }

  SDL_Window* _window;
  SDL_GLContext _glContext;
  Sdl::GlInitProc _glInit = nullptr;
  Sdl::GlResizeProc _glResize = nullptr;
  Sdl::GlDisplayProc _glDisplay = nullptr;
  bool _fullscreen = false;
  glm::ivec2 _windowSize = { 800, 600 };

#ifdef GLAD_DEBUG
  void glad_pre_callback(const char *, void *, int, ...) {}

  void glad_post_callback(const char *name, void *, int, ...) {
    GLenum err;
    err = glad_glGetError();
    if (err == GL_NO_ERROR)
      return;
    println(stderr, "GL error(s) occurred after calling {} with error #{}:", name, err);

    int numLogs;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numLogs);

    assert(numLogs > 0);

    char log[1024];
#if defined(_MSC_VER)
    auto sources = std::make_unique<GLenum[]>(numLogs);
    auto types = std::make_unique<GLenum[]>(numLogs);
    auto ids = std::make_unique<GLuint[]>(numLogs);
    auto severities = std::make_unique<GLenum[]>(numLogs);
    auto lengths = std::make_unique<GLsizei[]>(numLogs);
    glad_glGetDebugMessageLog(numLogs, sizeof(log), sources.get(), types.get(), ids.get(), severities.get(), lengths.get(), log);
#else
    GLenum sources[numLogs];
    GLenum types[numLogs];
    GLuint ids[numLogs];
    GLenum severities[numLogs];
    GLsizei lengths[numLogs];
    glad_glGetDebugMessageLog(numLogs, sizeof(log), sources, types, ids, severities, lengths, log);
#endif

    size_t off = 0;
    println("--- GL MESSAGE LOG BEGIN ---");
    for (int i = 0; i < numLogs; i++) {
      println(stderr, "{}", log + off);
      off += lengths[i];
    }
    //for (auto&& len : lengths) {
    //    println(stderr, "{}", log + off);
    //    off += len;
    //}
    println("---- GL MESSAGE LOG END ----");

    std::terminate();
  }
#endif //GLAD_DEBUG
}

void Sdl::setGlInit(GlInitProc glInit) {
  _glInit = glInit;
}

void Sdl::setGlResize(GlResizeProc glResize) {
  _glResize = glResize;
}

void Sdl::setGlDisplay(GlDisplayProc glDisplay) {
  _glDisplay = glDisplay;
}

void Sdl::mainLoop() {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    fatal("Error initialising SDL: {}", SDL_GetError());

  if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0)
    fatal("Error initialising SDL_image: {}", IMG_GetError());

  /* Print SDL version */
  SDL_version compiled, linked;
  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  println("SDL2 Header Version: {}", compiled);
  println("SDL2 Library Version: {}", linked);

  /* Specify a OpenGL 4.3 Core profile context */
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetSwapInterval(1);

  _window = SDL_CreateWindow("SDL2 Window",
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             _windowSize.x,
                             _windowSize.y,
                             SDL_WINDOW_OPENGL);

  if (!_window)
    fatal("Error creating SDL Window: {}", SDL_GetError());

  _glContext = SDL_GL_CreateContext(_window);
  if (!_glContext)
    fatal("Error creating OpenGL Context: {}", SDL_GetError());

  /* Init GL functions using GLAD */
  gladLoadGLLoader(SDL_GL_GetProcAddress);
  gladLoadGL();

#ifdef GLAD_DEBUG
  glad_set_pre_callback(glad_pre_callback);
  glad_set_post_callback(glad_post_callback);
#endif

  /* Print GL Version */
  println("GL Vendor: {}", glGetString(GL_VENDOR));
  println("GL Renderer: {}", glGetString(GL_RENDERER));
  println("GL Version: {}", glGetString(GL_VERSION));
  println("GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

  _glInit();
  for (;;) {
    /* Event loop */
    Update update;
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT:
          std::exit(0);
          break;

        case SDL_MOUSEBUTTONDOWN:
          if (ev.button.button & SDL_BUTTON_LMASK) {
            update.oX = ev.button.x;
            update.oY = ev.button.y;
#ifndef SPHERICAL_TRACKBALL
            SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
          }
          break;

        case SDL_MOUSEBUTTONUP:
          if (ev.button.button & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
            update.state = States::none;
          } else if (ev.button.button & (SDL_BUTTON_MMASK)) {
            update.state = States::reset;
          }
#ifndef SPHERICAL_TRACKBALL
          SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
          break;

        case SDL_MOUSEMOTION:
          if (ev.button.button & SDL_BUTTON_LMASK) {
            update.state = States::translate;
          } else if (ev.button.button & SDL_BUTTON_RMASK) {
            if (SDL_GetModState() & KMOD_SHIFT) {
              update.state = States::rotateLight;
            } else {
              update.state = States::rotate;
            }
          }

          update.x = ev.motion.xrel;
          update.y = ev.motion.yrel;
          break;

        case SDL_MOUSEWHEEL:
          update.state = States::zoom;
          update.x = ev.wheel.x;
          update.y = ev.wheel.y;

        case SDL_KEYUP:
          switch (ev.key.keysym.sym) {
            case SDLK_RETURN:
              if (ev.key.keysym.mod & KMOD_ALT) {
                _fullscreen = !_fullscreen;
                SDL_SetWindowFullscreen(_window, _fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                if (_glResize)
                  _glResize();
                update.state = States::fullScreen;
              }
              break;

            case SDLK_r:
              gRotateModel = !gRotateModel;
              break;

            case SDLK_s:
              gShaderMode = (gShaderMode + 1) % 3;
              break;

            case SDLK_m:
              gModel = (gModel + 1) % 3;
              break;

            case SDLK_w:
              gWaterized = !gWaterized;
              break;

            case SDLK_F1:
              gMoveLights = !gMoveLights;
              break;

            case SDLK_F2:
              gNumLights = (gNumLights + 1) % 3;
              break;

            case SDLK_F3:
              gSun = !gSun;
              break;

            case SDLK_F4:
              gSpotlight = !gSpotlight;
              break;

            case SDLK_SPACE:
              update.state = States::togglePerspective;
              break;

            case SDLK_EQUALS:
              gAmbient += 0.1f;
              if (gAmbient > 1.0f) {
                gAmbient = 1.0f;
              }
              break;

            case SDLK_MINUS:
              gAmbient -= 0.1f;
              if (gAmbient < 0.0f) {
                gAmbient = 0.0f;
              }
              break;

            case SDLK_ESCAPE:
              exit(0);
              break;

            default:
              break;
          }
          break;

        default:
          break;
      }
    }

    // Reset the last used shader program ID to 0
    gUseProgram = 0;

    _glDisplay(update);
    SDL_GL_SwapWindow(_window);

    /* Sleep for 10ms */
    // SDL_Delay(10);
  }
}

glm::ivec2 Sdl::mouseCoords() {
  glm::ivec2 rv;
  SDL_GetMouseState(&rv.x, &rv.y);
  return rv;
}

glm::ivec2 Sdl::screenCoords() {
  if (!_fullscreen)
    return _windowSize;

  glm::ivec2 rv;
  SDL_GL_GetDrawableSize(_window, &rv.x, &rv.y);
  return rv;
}
