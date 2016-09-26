#include "Renderer.hh"
#include "Object.hh"
#include "Shader.hh"
#include "Trackball.hh"
//#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
	Shader shader;

	Trackball trackball;

    Object object;

	bool modelTransforUnloaded = true;
}

void Renderer::checkAndLoadUniforms()
{
	if (modelTransforUnloaded) {
		shader.uniform("model") = glm::scale(glm::mat4(), glm::vec3(object.scaleFactor, object.scaleFactor, object.scaleFactor));
		modelTransforUnloaded = false;
	}

	if (trackball.viewDirty) {
		shader.uniform("view") = trackball.rotationMatrix();
		trackball.viewDirty = false;
	}

	if (trackball.projectionDirty) {
		shader.uniform("projection") = trackball.projectionMatrix();
		trackball.projectionDirty = false;
	}

	if (trackball.lightDirty) {
		shader.uniform("lightPos") = trackball.lightPosition();
		trackball.lightDirty = false;
	}
}

void Renderer::init()
{
    object.load("grieghallen");
	object.scaleFactor = 0.02f;

//    glGenBuffers(1, &vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), cube, GL_STATIC_DRAW);
//
//    glGenBuffers(1, &ibo);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(GLuint), cubeIndices, GL_STATIC_DRAW);
//
//    glGenVertexArrays(1, &vao);
//    glBindVertexArray(vao);

	shader.load("cube");

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::draw(Update update)
{
	if (update.oX > 0 || update.oY > 0) {
		trackball.anchorRotation(update.oX, update.oY);
	}

	switch (update.state) {
    case States::reset:
        trackball.reset();
		break;

    case States::rotate:
        trackball.rotate(update.x, update.y);
        break;

    case States::rotateLight:
        trackball.rotateLight(update.x, update.y);
        break;

    case States::translate:
        trackball.translate(update.x, update.y);
		break;

    case States::zoom:
        trackball.zoom(update.y);
		break;

	case States::togglePerspective:
		trackball.togglePerspective();
		break;

    default:
		break;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();
    object.bind();
	checkAndLoadUniforms();
    object.draw();
	glDisableVertexAttribArray(0);
	glUseProgram(0);

//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
//    glBindVertexArray(vao);
//
//    checkAndLoadUniforms();
//
//    glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
//    glDisableVertexAttribArray(0);
//    glUseProgram(0);
}
