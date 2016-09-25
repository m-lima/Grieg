#include <SDL_timer.h>
#include "renderer.hh"
#include "sdl_gl.hh"
#include "shader.hh"

namespace Renderer {

	GLuint vbo = 0; // Vertex Buffer Object
	GLuint vao = 0; // Vertex Array Object

	Shader shader;

	static constexpr GLfloat points[] = {
		.0f, .5f, .0f,
		.5f, -.5f, .0f,
		-.5f, -.5f, .0f,
	};

	void loadVertices()
	{
	}

	void loadProgram()
	{
	}

	void checkAndLoadUniforms()
	{
	}

	void init()
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

	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT);

		shader.uniform("fGlobalTime") = SDL_GetTicks() / 1000.0f;
		shader.use();
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}