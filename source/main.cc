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

    GLuint vbo = 0; // Vertex Buffer Object
    GLuint vao = 0; // Vertex Array Object
    GLuint shader_program = 0;

    GLfloat points[] = {
            .0f, .5f, .0f,
            .5f, -.5f, .0f,
            -.5f, -.5f, .0f,
    };

    const char *vertex_shader_code =
            "#version 440\n"
                    "in vec3 vp;\n"
                    "void main() {\n"
                    "  gl_Position = vec4(vp, 1.0);\n"
                    "}";

    const char *fragment_shader_code =
            "#version 440\n"
                    "out vec4 frag_color;\n"
                    "void main() {\n"
                    "  frag_color = vec4(0.5, 0.0, 0.5, 1.0);\n"
                    "}";

    void initGL()
    {
        GLint success = 0;

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertex_shader_code, nullptr);
        glCompileShader(vs);
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetShaderInfoLog(vs, sizeof log, nullptr, log);
            fmt::fatal("Couldn't compile vertex shader\nLog:\n{}", log);
        }

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragment_shader_code, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetShaderInfoLog(fs, sizeof log, nullptr, log);
            fmt::fatal("Couldn't compile fragment shader\nLog:\n{}", log);
        }

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vs);
        glAttachShader(shader_program, fs);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetProgramInfoLog(shader_program, sizeof log, nullptr, log);
            fmt::fatal("Couldn't link shader program\nLog:\n{}", log);
        }

        // glDeleteShader(vs);
        // glDeleteShader(fs);

        glClearColor(0, 0, 0, 1);
    }

    void displayGL()
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
        fatal("SDL Error: {}", SDL_GetError());

    print_sdl_info();

    SDL_Window *window;
    SDL_GLContext context;

    window = SDL_CreateWindow("INF251", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    gladLoadGL();

    print_gl_info();

    initGL();

    bool fullscreen = false;
    for (;;)
    {
        SDL_Event ev;
        while(SDL_PollEvent(&ev))
        {
            switch (ev.type) {
                case SDL_QUIT:
                    std::exit(0);
                    break;

                case SDL_KEYDOWN:
                    if (ev.key.keysym.mod & KMOD_ALT && ev.key.keysym.sym == SDLK_RETURN)
                    {
                        fullscreen = !fullscreen;
                        SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    }
                    break;

                default:
                    break;
            }
        }

        displayGL();

        SDL_GL_SwapWindow(window);
        SDL_Delay(10);
    }
}
