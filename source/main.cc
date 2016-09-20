#include "sdl_gl.hh"

namespace {
    GLuint vbo = 0; // Vertex Buffer Object
    GLuint vao = 0; // Vertex Array Object
    GLuint shader_program = 0;

    GLfloat points[] = {
            .0f, .5f, .0f,
            .5f, -.5f, .0f,
            -.5f, -.5f, .0f,
    };

    const char *vertex_shader_code =
            "#version 430\n"
                    "in vec3 vp;\n"
                    "void main() {\n"
                    "  gl_Position = vec4(vp, 1.0);\n"
                    "}";

    const char *fragment_shader_code =
            "#version 430\n"
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
    Sdl sdl;

    sdl.set_glinit(initGL);
    sdl.set_gldisplay(displayGL);
    sdl.main_loop();
}
