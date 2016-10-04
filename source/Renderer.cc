#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.hh"
#include "Object.hh"
#include "Shader.hh"
#include "Trackball.hh"
#include "Texture.hh"

namespace
{
  Shader shader;
  Shader gridShader;

  Trackball trackball;

  Object object;

  Texture texture;
  GLuint gridVbo = 0;
  GLuint gridVao = 0;

  std::vector<Shader*> mvpShaders;

  template <class T>
  void setUniform(const std::string &name, T&& value) {
      for (auto&& sh: mvpShaders) {
          sh->use();
          sh->uniform(name) = std::forward<T>(value);
      }
  }
}

void Renderer::checkAndLoadUniforms()
{
    if (object.modelDirty) {
        setUniform("model", object.modelTransform);
        object.modelDirty = false;
    }

    if (trackball.viewDirty) {
        setUniform("view", trackball.rotation());
        shader.uniform("sunPos") = static_cast<Quat>(glm::inverse(trackball.rotation())) * glm::normalize(Vec3(1.0f));
        trackball.viewDirty = false;
    }

    if (trackball.projectionDirty) {
        setUniform("projection", trackball.projection());
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
    object.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

    shader.load("cube");
    gridShader.load("grid");

    mvpShaders.push_back(&shader);
    mvpShaders.push_back(&gridShader);

    //texture.load("Mollweide", "jpg");

    /* Create grid quad */
    constexpr float gridSize = 1000.0f;
    const glm::vec3 gridQuad[] = {
        { -gridSize, 0.0f, -gridSize },
        {  gridSize, 0.0f, -gridSize },
        {  gridSize, 0.0f,  gridSize },
        { -gridSize, 0.0f,  gridSize }
    };
    glGenVertexArrays(1, &gridVao);
    glBindVertexArray(gridVao);

    glGenBuffers(1, &gridVbo);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(gridQuad),
                 &gridQuad[0],
                 GL_STATIC_DRAW);

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

    checkAndLoadUniforms();

    /* Draw grid before doing anything else */
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    gridShader.use();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat) * 3, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(0);
    glEnable(GL_DEPTH_TEST);

    shader.use();
    object.bind();
    object.draw();
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glUseProgram(0);
}
