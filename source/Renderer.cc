#include "Renderer.hh"
#include "Sdl.hh"
#include "Shader.hh"
#include "Trackball.hh"

namespace
{
    GLuint vbo = 0; // Vertex Buffer Object
    GLuint vao = 0; // Vertex Array Object
    GLuint ibo = 0; // Index Buffer Object

    Shader shader;

    Trackball trackball;

    constexpr GLuint cubeIndices[] = {
        1, 5, 7,
        7, 3, 1,
        0, 2, 6,
        6, 4, 0,
        0, 1, 3,
        3, 2, 0,
        7, 5, 4,
        4, 6, 7,
        2, 3, 7,
        7, 6, 2,
        1, 0, 4,
        4, 5, 1
    };

    constexpr GLfloat cube[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f
    };
}

void Renderer::loadVertices()
{
}

void Renderer::loadProgram()
{
}

void Renderer::checkAndLoadUniforms()
{
    if (trackball.viewDirty) {
        shader.uniform("view") = trackball.rotationMatrix();
        trackball.viewDirty = false;
    }

    if (trackball.projectionDirty) {
        shader.uniform("projection") = trackball.projectionMatrix();
        trackball.projectionDirty = false;
    }
}

void Renderer::init()
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), cube, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(GLuint), cubeIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    shader.load("cube");

    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::draw(Update update)
{
    switch (update.state) {
    case States::start: trackball.mousePressed(update.x, update.y);
        break;
    case States::rotate: trackball.rotate(update.x, update.y);
        break;
    case States::translate: trackball.translate(update.x, update.y);
        break;
    case States::zoom: trackball.zoom(update.x, update.y);
        break;
    case States::fov: trackball.fov(update.x, update.y);
        break;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindVertexArray(vao);

    checkAndLoadUniforms();

    glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glUseProgram(0);
}
