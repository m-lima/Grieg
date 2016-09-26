#include <memory>
#include <stdlib.h>
#include "SDL.h"
#include "Sdl.hh"

extern GLuint gUseProgram;

namespace {
	std::ostream& operator<<(std::ostream& s, const SDL_version &v)
	{
		return s << format("{:d}.{:d}.{:d}", v.major, v.minor, v.patch);
	}

	SDL_Window* window;
	SDL_GLContext glContext;
	Sdl::GlInitProc _glInit;
	Sdl::GlDisplayProc _glDisplay;

#ifdef GLAD_DEBUG
	void glad_pre_callback(const char *name, void *, int, ...) {
	}

	void glad_post_callback(const char *name, void *, int, ...) {
		GLenum err;
		err = glad_glGetError();
		if (err == GL_NO_ERROR)
			return;
		println(stderr, "GL error(s) occurred after calling {}:", name, err);

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
		glad_glGetDebugMessageLog(1, sizeof(log), sources.get(), types.get(), ids.get(), severities.get(), lengths.get(), log);
#else
		GLenum sources[numLogs];
		GLenum types[numLogs];
		GLuint ids[numLogs];
		GLenum severities[numLogs];
		GLsizei lengths[numLogs];
		glad_glGetDebugMessageLog(1, sizeof(log), sources, types, ids, severities, lengths, log);
#endif
		println(stderr, "{}", log);

		std::terminate();
	}
#endif //GLAD_DEBUG
}

void Sdl::setGlInit(GlInitProc glInit)
{
	_glInit = glInit;
}

void Sdl::setGlDisplay(GlDisplayProc glDisplay)
{
	_glDisplay = glDisplay;
}

void Sdl::mainLoop()
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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("SDL2 Window",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800,
		600,
		SDL_WINDOW_OPENGL);

	if (!window)
		fatal("Error creating SDL Window: {}", SDL_GetError());

	glContext = SDL_GL_CreateContext(window);
	if (!glContext)
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
				if (update.state == States::none) {
					if (ev.button.button & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
						update.state = States::start;
						update.x = ev.button.x;
						update.y = ev.button.y;
						SDL_SetRelativeMouseMode(SDL_TRUE);
					}
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if (ev.button.button & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
					update.state = States::none;
				}
				else if (ev.button.button & (SDL_BUTTON_MMASK)) {
					update.state = States::reset;
				}
				SDL_SetRelativeMouseMode(SDL_FALSE);
				break;

			case SDL_MOUSEMOTION:
				if (ev.button.button & SDL_BUTTON_LMASK)
					update.state = States::translate;
				else if (ev.button.button & SDL_BUTTON_RMASK)
					update.state = States::rotate;

				update.x = ev.motion.xrel;
				update.y = ev.motion.yrel;
				break;

			case SDL_MOUSEWHEEL:
				if (SDL_GetModState() & KMOD_CTRL)
					update.state = States::fov;
				else
					update.state = States::zoom;
				update.x = ev.wheel.x;
				update.y = ev.wheel.y;

			default:
				break;
			}
		}

		// Reset the last used shader program ID to 0
		gUseProgram = 0;

		_glDisplay(update);
		SDL_GL_SwapWindow(window);

		/* Sleep for 10ms */
		SDL_Delay(10);
	}
}

glm::ivec2 Sdl::mouseCoords()
{
    glm::ivec2 rv;
    SDL_GetMouseState(&rv.x, &rv.y);
	return rv;
}

glm::ivec2 Sdl::screenCoords()
{
	glm::ivec2 rv;
    SDL_GetWindowSize(window, &rv.x, &rv.y);
	return rv;
}