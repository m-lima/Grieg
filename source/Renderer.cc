#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.hh"
#include "Object.hh"
#include "Shader.hh"
#include "Trackball.hh"
#include "Texture.hh"
#include "Text.hh"
#include "ShaderStorage.hh"

bool gSun = true;
int gNumLights = 2;
bool gMoveLights = true;
bool gSpotlight = false;
float gAmbient = 0.2f;
bool gRotateModel = false;

namespace {
  auto shader = std::make_shared<Shader>();
  auto shader2 = std::make_shared<Shader>();
  auto gridShader = std::make_shared<Shader>();

  Trackball trackball;

  Object grieghallen;
  Object suzanne1;
  Object suzanne2;

  Texture texture;
  GLuint gridVbo = 0;
  GLuint gridVao = 0;

  uint32_t fpsLast = 0;
  uint32_t fpsCount = 0;
  Text fpsText({ 0, 0 });

  Text usageText;

  GLuint frameBuffer;
  GLuint frameBufferTexture;

  struct MatrixBlock {
    static constexpr auto name = "MatrixBlock";
    static constexpr auto binding = 0;

    glm::mat4 proj;
    glm::mat4 view;
  };

  float _lightAngle{};
  float _lightTilt{};
  float _tiltFactor{ 0.01f };

  ShaderStorage<MatrixBlock> matrixBuffer;

  struct LightBlock {
    static constexpr auto name = "LightBlock";
    static constexpr auto binding = 1;

    int type;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 position;
    float specularIndex = 256.0f;
    float aperture;
    float intensity = 1.0f;
  };

  ShaderStorage<LightBlock[], 12> lightBuffer;

  void generateFrameBuffer() {
    glGenTextures(1, &frameBufferTexture);
    glActiveTexture(GL_TEXTURE0 + frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    auto screen = Sdl::screenCoords();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen.x, screen.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

  }
}

void Renderer::checkAndLoadUniforms() {
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

void Renderer::init() {

  generateFrameBuffer();

  gridShader->load("grid");
  gridShader->bindBuffer(matrixBuffer);

  shader->load("normals");
  shader->bindBuffer(matrixBuffer);
  shader->bindBuffer(lightBuffer);
  shader->uniform("uTexture") = Sampler2D(0);

  shader2->load("toon");
  shader2->bindBuffer(matrixBuffer);
  shader2->bindBuffer(lightBuffer);
  shader2->uniform("uTexture") = Sampler2D(0);
  shader2->uniform("uFramebuffer") = Sampler2D(frameBufferTexture);

  grieghallen.load("grieghallen");
  grieghallen.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));
  grieghallen.setShader(shader);
  grieghallen.haveTexture = true;

  suzanne1.load("suzanne");
  suzanne1.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));
  suzanne1.setShader(shader);

  suzanne2.load("suzanne");
  suzanne2.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));
  suzanne2.setShader(shader);

  Text::setGlobalFont(Texture::cache("font.png"));

  usageText.setPosition({ 1, 47 - 11 });
  usageText.format(
    "Left mouse:   Translate\n"
    "Right mouse:  Rotate\n"
    "Middle mouse: Reset view\n"
    "Spacebar:     Toggle perspective\n"
    "R:            Toggle model rotation\n"
    "S:            Cycle shader\n"
    "F1:           Toggle light movement\n"
    "F2:           Change number of lights\n"
    "F3:           Toggle sun\n"
    "F4:           Toggle spotlight/point light\n"
    "-/+:          Darken/brighten ambient light\n");

  /* Create lights */
  lightBuffer[0].type = 1; /* Directional */
  lightBuffer[0].color = { 1.0f, 1.0f, 1.0f };
  lightBuffer[0].position = { 0.0, 10.0f, 0.0f };

  lightBuffer[1].type = 2; /* Point light */
  lightBuffer[1].color = { 0.0f, 0.0f, 1.0f };
  lightBuffer[1].direction = { 1.0f, 0.0f, 0.0f };
  lightBuffer[1].aperture = 0.2f;

  lightBuffer[2].type = 2; /* Point light */
  lightBuffer[2].color = { 0.0f, 1.0f, 0.0f };
  lightBuffer[2].intensity = 0.5f;
  lightBuffer[2].aperture = 0.4f;
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

void Renderer::resize() {
  glViewport(0, 0, Sdl::screenCoords().x, Sdl::screenCoords().y);
}

void Renderer::draw(Update update) {
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
    auto &direction = lightBuffer[0].direction;
    direction = {
      cos(_lightAngle) * sin(_lightAngle),
      cos(_lightAngle),
      sin(_lightAngle) * sin(_lightAngle) };
    direction *= 10.0f;
    lightBuffer[0].type = (gSun) ? 1 : 0;
    lightBuffer.update();
  }

  {
    auto &position = lightBuffer[1].position;
    position = { cos(_lightAngle), 0.0f, sin(_lightAngle) };
    position *= 70.0f + 25.0f * sin(_lightAngle);
    lightBuffer[1].type = (gNumLights >= 1) ? (gSpotlight ? 3 : 2) : 0;
    lightBuffer[1].direction = glm::normalize(-position);
    lightBuffer[1].direction.y -= _lightTilt;
    lightBuffer.update();
    suzanne1.setPosition(position / 20.0f);
  }

  {
    auto &position = lightBuffer[2].position;
    position = { cos(-_lightAngle), 0.0f, sin(-_lightAngle) };
    position *= 50.0f;
    lightBuffer[2].type = (gNumLights >= 2) ? (gSpotlight ? 3 : 2) : 0;
    lightBuffer[2].direction = glm::normalize(-position);
    lightBuffer[2].direction.y += _lightTilt;
    lightBuffer.update();
    suzanne2.setPosition(position / 20.0f);
  }

  if (gMoveLights) {
    _lightAngle += 0.005f;
  }

  _lightTilt += _tiltFactor;
  if (_lightTilt > 1.0f || _lightTilt < -1.0f) {
    _tiltFactor *= -1.0f;
  }

  if (gRotateModel) {
    grieghallen.modelTransform = glm::rotate(
      grieghallen.modelTransform, 0.01f, glm::vec3(0, 1, 0));
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  checkAndLoadUniforms();

  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  grieghallen.setShader(shader);
  suzanne1.setShader(shader);
  suzanne2.setShader(shader);

  /* Draw grid before doing anything else */
  //glDisable(GL_DEPTH_TEST);
  //glBindVertexArray(gridVao);
  //glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
  //gridShader->use();
  //glEnableVertexAttribArray(0);
  //glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat) * 3, 0);
  //glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  //glDisableVertexAttribArray(0);
  //glEnable(GL_DEPTH_TEST);

  shader->uniform("uAmbientLight") = glm::vec3(gAmbient);
  shader2->uniform("uAmbientLight") = glm::vec3(gAmbient);
  shader2->uniform("uScreenSize") = glm::vec2(Sdl::screenCoords().x, Sdl::screenCoords().y);

  grieghallen.draw();
  if (gNumLights >= 1)
    suzanne1.draw();
  if (gNumLights >= 2)
    suzanne2.draw();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  grieghallen.setShader(shader2);
  suzanne1.setShader(shader2);
  suzanne2.setShader(shader2);
  glActiveTexture(GL_TEXTURE0 + frameBufferTexture);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

  grieghallen.draw();
  if (gNumLights >= 1)
    suzanne1.draw();
  if (gNumLights >= 2)
    suzanne2.draw();

  glDisable(GL_DEPTH_TEST);
  Text::drawAll();
  glEnable(GL_DEPTH_TEST);
  glUseProgram(0);

  if ((SDL_GetTicks() - fpsLast) >= 1000) {
    fpsText.format("FPS: {}", fpsCount);
    fpsCount = 0;
    fpsLast = SDL_GetTicks();
  }
  fpsCount++;
}
