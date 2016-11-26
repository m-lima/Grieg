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
bool gSpotlight = true;
float gAmbient = 0.4f;
bool gRotateModel = false;
int gShaderMode = 0;
int gModel = 0;
bool gWaterized = false;

namespace {

  constexpr int TEXTURE_LOCATION = 0;
  constexpr int BUMP_LOCATION = 2;
  constexpr int FRAMEBUFFER_LOCATION = 10;
  constexpr int NORMALBUFFER_LOCATION = 11;
  constexpr int DEPTHBUFFER_LOCATION = 12;

  auto basicShader = std::make_shared<Shader>();
  auto toonShader = std::make_shared<Shader>();
  auto depthShader = std::make_shared<Shader>();
  auto gridShader = std::make_shared<Shader>();

  Trackball trackball;

  Object grieghallen;
  Object suzanne1;
  Object suzanne2;
  Object bigSuzy;
  Object terrain;

  auto water = std::make_shared<Texture>();
  auto bump = std::make_shared<Texture>();
  auto bergen = std::make_shared<Texture>();
  Texture texture;
  GLuint gridVbo = 0;
  GLuint gridVao = 0;

  uint32_t fpsLast = 0;
  uint32_t fpsCount = 0;
  Text fpsText({ 0, 0 });

  Text usageText;

  GLuint frameBuffer;
  GLuint frameBufferTexture;
  GLuint normalBufferTexture;
  GLuint depthBufferTexture;

  bool currentWaterized = false;

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

  void generateFrameBuffer(int width, int height) {
    // Color attachment
    gl.glGenTextures(1, &frameBufferTexture);
    gl.glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
    gl.glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Normal attachment
    gl.glGenTextures(1, &normalBufferTexture);
    gl.glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
    gl.glBindTexture(GL_TEXTURE_2D, normalBufferTexture);

    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Depth attachment
    gl.glGenTextures(1, &depthBufferTexture);
    gl.glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
    gl.glBindTexture(GL_TEXTURE_2D, depthBufferTexture);

    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    // Actual frame buffer
    gl.glGenFramebuffers(1, &frameBuffer);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalBufferTexture, 0);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    gl.glDrawBuffers(2, attachments);

    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void drawAll() {
    switch (gModel) {
      case 0:
        grieghallen.draw();
        break;

      case 1:
        if (gWaterized) {
          if (!currentWaterized) {
            bigSuzy.setMaterial(water);
            currentWaterized = true;
          }
        } else {
          if (currentWaterized) {
            bigSuzy.setBump(bump);
            currentWaterized = false;
          }
        }

        bigSuzy.draw();
        break;

      case 2:
        terrain.draw();
        break;
    }

    if (gNumLights >= 1)
      suzanne1.draw();
    if (gNumLights >= 2)
      suzanne2.draw();
  }

  void setAllShaders(std::shared_ptr<Shader> shader) {
    grieghallen.setShader(shader);
    suzanne1.setShader(shader);
    suzanne2.setShader(shader);
    bigSuzy.setShader(shader);
    terrain.setShader(shader);
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

void Renderer::init(int width, int height) {

  generateFrameBuffer(width, height);

  water->load("water.jpg", 16);

  gridShader->load("grid");
  gridShader->bindBuffer(matrixBuffer);

  basicShader->load("basic");
  basicShader->bindBuffer(matrixBuffer);
  basicShader->bindBuffer(lightBuffer);
  basicShader->uniform("uTexture") = Sampler2D(TEXTURE_LOCATION);
  basicShader->uniform("uBump") = Sampler2D(BUMP_LOCATION);

  toonShader->load("toon");
  toonShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  toonShader->uniform("uNormalbuffer") = Sampler2D(NORMALBUFFER_LOCATION);
  toonShader->uniform("uDepthbuffer") = Sampler2D(DEPTHBUFFER_LOCATION);
  toonShader->uniform("uScreenSize") = glm::vec2(width, height);

  depthShader->load("depth");
  depthShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  depthShader->uniform("uScreenSize") = glm::vec2(width, height);

  grieghallen.load("grieghallen");
  grieghallen.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  suzanne1.load("suzanne");
  suzanne1.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  suzanne2.load("suzanne");
  suzanne2.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  bigSuzy.load("suzanne");

  terrain.load("bergen_1024x918");
  //terrain.load("bergen_2048x1836");
  //terrain.load("bergen_3072x2754");
  terrain.modelTransform = glm::scale(Mat4(), Vec3(4.0f, 4.0f, 4.0f));

  bergen->load("bergen_terrain_texture.png");
  terrain.setMaterial(bergen);

  bump->load("Rock.jpg");
  bigSuzy.setBump(bump);

  Text::setGlobalFont(Texture::cache("font.png"));

  usageText.setPosition({ 1, 47 - 13 });
  usageText.format(
    "Left mouse:   Translate\n"
    "Right mouse:  Rotate\n"
    "Middle mouse: Reset view\n"
    "Spacebar:     Toggle perspective\n"
    "R:            Toggle model rotation\n"
    "S:            Cycle shader\n"
    "M:            Change model\n"
    "W:            Waterize\n"
    "F1:           Toggle light movement\n"
    "F2:           Change number of lights\n"
    "F3:           Toggle sun\n"
    "F4:           Toggle spotlight/point light\n"
    "-/+:          Darken/brighten ambient light\n");

  /* Create lights */
  lightBuffer[0].type = 1;
  lightBuffer[0].color = { 1.0f, 1.0f, 1.0f };
  lightBuffer[0].position = { 0.0, 10.0f, 0.0f };

  lightBuffer[1].type = 3;
  lightBuffer[1].color = { 0.0f, 0.0f, 1.0f };
  lightBuffer[1].direction = { 1.0f, 0.0f, 0.0f };
  lightBuffer[1].aperture = 0.01f;

  lightBuffer[2].type = 3;
  lightBuffer[2].color = { 0.0f, 1.0f, 0.0f };
  lightBuffer[2].intensity = 0.5f;
  lightBuffer[2].aperture = 0.1f;
  lightBuffer.update();

  /* Create grid quad */
  constexpr float gridSize = 1000.0f;
  const glm::vec3 gridQuad[] = {
      { -gridSize, 0.0f, -gridSize },
      {  gridSize, 0.0f, -gridSize },
      {  gridSize, 0.0f,  gridSize },
      { -gridSize, 0.0f,  gridSize }
  };
  gl.glGenVertexArrays(1, &gridVao);
  gl.glBindVertexArray(gridVao);

  gl.glGenBuffers(1, &gridVbo);
  gl.glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
  gl.glBufferData(GL_ARRAY_BUFFER,
               sizeof(gridQuad),
               &gridQuad[0],
               GL_STATIC_DRAW);

  gl.glClearColor(0, 0, 0, 1);
  gl.glEnable(GL_CULL_FACE);
  gl.glCullFace(GL_BACK);
  gl.glEnable(GL_DEPTH_TEST);
  gl.glEnable(GL_BLEND);
  gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gl.glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  gl.glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
  gl.glActiveTexture(GL_TEXTURE0 + NORMALBUFFER_LOCATION);
  gl.glBindTexture(GL_TEXTURE_2D, normalBufferTexture);
  gl.glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
  gl.glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
}

void Renderer::resize(int width, int height) {

  gl.glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  gl.glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
  gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  gl.glActiveTexture(GL_TEXTURE0 + NORMALBUFFER_LOCATION);
  gl.glBindTexture(GL_TEXTURE_2D, normalBufferTexture);
  gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  gl.glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
  gl.glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
  gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  gl.glViewport(0, 0, width, height);

  toonShader->uniform("uScreenSize") = glm::vec2(width, height);
  depthShader->uniform("uScreenSize") = glm::vec2(width, height);
}

void Renderer::draw() {
  //if (update.oX > 0 || update.oY > 0) {
  //  trackball.anchorRotation(update.oX, update.oY);
  //}

  //switch (update.state) {
  //  case States::reset:
  //    trackball.reset();
  //    break;

  //  case States::rotate:
  //    trackball.rotate(update.x, update.y);
  //    break;

  //  case States::rotateLight:
  //    trackball.rotateLight(update.x, update.y);
  //    break;

  //  case States::translate:
  //    trackball.translate(update.x, update.y);
  //    break;

  //  case States::zoom:
  //    trackball.zoom(update.y);
  //    break;

  //  case States::togglePerspective:
  //    trackball.togglePerspective();
  //    break;

  //  case States::fullScreen:
  //    trackball.projectionDirty = true;
  //    break;

  //  default:
  //    break;
  //}

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
    bigSuzy.modelTransform = glm::rotate(
      bigSuzy.modelTransform, 0.01f, glm::vec3(0, 1, 0));
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  checkAndLoadUniforms();

  /* Draw grid before doing anything else */
  gl.glDisable(GL_DEPTH_TEST);
  gl.glDisable(GL_CULL_FACE);
  gl.glBindVertexArray(gridVao);
  gl.glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
  gridShader->use();
  gl.glEnableVertexAttribArray(0);
  gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat) * 3, 0);
  gl.glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  gl.glDisableVertexAttribArray(0);
  gl.glEnable(GL_CULL_FACE);
  gl.glEnable(GL_DEPTH_TEST);

  setAllShaders(basicShader);
  basicShader->uniform("uAmbientLight") = glm::vec3(gAmbient);

  switch (gShaderMode) {
    case 1:
      grieghallen.enableTexture = false;
      bigSuzy.enableTexture = false;
      terrain.enableTexture = false;

      gl.glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      drawAll();
      gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

      setAllShaders(toonShader);
      grieghallen.enableTexture = true;
      bigSuzy.enableTexture = true;
      terrain.enableTexture = true;
      break;

    case 2:
      gl.glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
      gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      drawAll();
      gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

      setAllShaders(depthShader);
      break;
  }

  drawAll();

  gl.glDisable(GL_DEPTH_TEST);
  Text::drawAll();
  gl.glEnable(GL_DEPTH_TEST);
  gl.glUseProgram(0);

  //if ((SDL_GetTicks() - fpsLast) >= 1000) {
  //  fpsText.format("FPS: {}", fpsCount);
  //  fpsCount = 0;
  //  fpsLast = SDL_GetTicks();
  //}
  fpsCount++;
}
