#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.hh"
#include "Object.hh"
#include "Shader.hh"
#include "Trackball.hh"
#include "Texture.hh"
#include "Text.hh"
#include "UniformBuffer.hh"

bool gMoveLights = false;

namespace
{
  auto shader = std::make_shared<Shader>();
  auto gridShader = std::make_shared<Shader>();

  Trackball trackball;

  Object grieghallen;

  Texture texture;
  GLuint gridVbo = 0;
  GLuint gridVao = 0;

  uint32_t fpsLast = 0;
  uint32_t fpsCount = 0;
  Text fpsText({0, 0});

  Text usageText;

  struct MatrixBlock {
      glm::mat4 proj;
      glm::mat4 view;
  };

  float _lightAngle {};

  UniformBuffer<MatrixBlock, 0> matrixBuffer("MatrixBlock");

  struct LightBlock {
      int type;
      int _pad0[3];
      glm::vec3 position;
      float _pad1;
      glm::vec3 normal;
      float _pad2;
      glm::vec3 color;
      float _pad3;
      float intensity;
      float _pad4[3];
  };

  UniformBuffer<LightBlock, 1, 12> lightBuffer("LightBlock");
}

void Renderer::checkAndLoadUniforms()
{
    if (trackball.viewDirty) {
        matrixBuffer->view = trackball.rotation();
        matrixBuffer.update();
        trackball.viewDirty = false;
    }

    if (trackball.projectionDirty) {
        matrixBuffer->proj = trackball.projection();
        matrixBuffer.update();
        trackball.projectionDirty = false;
    }
}

void Renderer::init()
{
    grieghallen.load("grieghallen");
    grieghallen.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

    shader->load("cube");
    shader->bindBuffer(matrixBuffer);
    shader->bindBuffer(lightBuffer);

    gridShader->load("grid");
    gridShader->bindBuffer(matrixBuffer);

    shader->uniform("uTexture") = Sampler2D(0);

    grieghallen.setShader(shader);
    grieghallen.update();

    Text::setGlobalFont(Texture::cache("font.png"));

    usageText.setPosition({0, 48 - 5});
    usageText.format(
        "Left mouse:   Translate\n"
        "Right mouse:  Rotate\n"
        "Middle mouse: Reset view\n"
        "Spacebar:     Toggle perspective\n"
        "F1:           Toggle light movement");

    /* Create lights */
    lightBuffer[0].type = 2;
    lightBuffer[0].color = { 1.0, 0.0, 0.0 };
    lightBuffer[1].type = 2;
    lightBuffer[1].color = { 0.0, 1.0, 0.0 };
    lightBuffer.update();

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::resize()
{
    glViewport(0, 0, Sdl::screenCoords().x, Sdl::screenCoords().y);
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

    case States::fullScreen:
        trackball.projectionDirty = true;
        break;

    default:
        break;
    }

    {
        auto &position = lightBuffer[0].position;
        position = {cos(_lightAngle), 0.0f, sin(_lightAngle)};
        position *= 500.0f;
        lightBuffer[0].normal = -position;
        lightBuffer.update();
    }

    {
        auto &position = lightBuffer[1].position;
        position = {cos(-_lightAngle), 0.0f, sin(-_lightAngle)};
        position *= 500.0f;
        lightBuffer[1].normal = -position;
        lightBuffer.update();
    }

    if (gMoveLights)
        _lightAngle += 0.02;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    checkAndLoadUniforms();

    /* Draw grid before doing anything else */
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    gridShader->use();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat) * 3, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(0);
    glEnable(GL_DEPTH_TEST);

    grieghallen.draw();

    glDisable(GL_DEPTH_TEST);
    Text::drawAll();
    glEnable(GL_DEPTH_TEST);

    if ((SDL_GetTicks() - fpsLast) >= 1000) {
        fpsText.format("FPS: {}", fpsCount);
        fpsCount = 0;
        fpsLast = SDL_GetTicks();
    }
    fpsCount++;
}
