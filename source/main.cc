#include <fstream>
#include <SDL_timer.h>
#include "sdl_gl.hh"
#include "shader.hh"
#include "trackball.hh"

namespace {
  GLuint vbo = 0; // Vertex Buffer Object
  GLuint vao = 0; // Vertex Array Object

  Shader shader;

  GLfloat points[] = {
      .0f, .5f, .0f,
      .5f, -.5f, .0f,
      -.5f, -.5f, .0f,
  };

  void initGL()
  {
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      shader.load("triangle");
      shader.uniform("fResolution") = { 640, 480 };

      glClearColor(0, 0, 0, 1);
  }

  void displayGL()
  {
      glClear(GL_COLOR_BUFFER_BIT);

      shader.uniform("fGlobalTime") = SDL_GetTicks() / 1000.0f;
      shader.use();
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
  }
}

std::string get_file_contents(const std::string &file)
{
    std::ifstream fh(file, std::ios::binary);
    if (!fh.is_open())
        throw std::runtime_error(format("Couldn't open file {}", file));

    std::string buf;
    size_t length;

    fh.seekg(0, std::ios::end);
    length = fh.tellg();
    buf.reserve(length);
    fh.seekg(0, std::ios::beg);
    buf.assign(std::istreambuf_iterator<char>(fh), std::istreambuf_iterator<char>());

    return buf;
}

int main()
{
    Sdl sdl;

    sdl.set_glinit(initGL);
    sdl.set_gldisplay(displayGL);
    sdl.main_loop();
}
