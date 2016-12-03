#include "Renderer.hh"

#include <QOpenGLFramebufferObject>
#include <QElapsedTimer>
#include <QMouseEvent>

#include "LightDialog.hh"

namespace {

  constexpr int TEXTURE_LOCATION = 0;
  constexpr int BUMP_LOCATION = 2;
  constexpr int FRAMEBUFFER_LOCATION = 10;
  constexpr int NORMALBUFFER_LOCATION = 11;
  constexpr int DEPTHBUFFER_LOCATION = 12;
  constexpr int LINEARDEPTHBUFFER_LOCATION = 13;

  bool moveLights = true;
  float ambientLevel = 0.4f;
  bool rotateModel = false;
  int currentModel = 0;
  bool waterized = false;
  bool currentWaterized = false;

  GLuint gridVbo = 0;
  GLuint gridVao = 0;

  GLuint frameBuffer;
  GLuint frameBufferTexture;
  GLuint normalBufferTexture;
  GLuint depthBufferTexture;
  GLuint linearDepthBufferTexture;

  float _lightAngle{};
  float _lightTilt{};
  float _tiltFactor{ 0.01f };

  QElapsedTimer timer;
  std::string fpsText = "FPS: 0";
  uint32_t fpsCount = 0;
}

Renderer::Renderer(QWidget *parent) :
  QOpenGLWidget(parent)
{
  basicShader = std::make_shared<Shader>();
  toonShader = std::make_shared<Shader>();
  depthShader = std::make_shared<Shader>();
  gridShader = std::make_shared<Shader>();

  water = std::make_shared<Texture>();
  bump = std::make_shared<Texture>();
  bergen = std::make_shared<Texture>();
}

void Renderer::checkAndLoadUniforms() {
  if (camera.viewDirty || camera.moving()) {
    matrixBuffer->view = camera.rotation();
    matrixBuffer.update();
    camera.viewDirty = false;

    Vec3 position = camera.eyePosition();
    auto strPos = fmt::format("X:{} Y:{} Z:{}", position.x, position.y, position.z);
    lblPosition->setText(strPos.c_str());
  }

  if (camera.projectionDirty) {
    matrixBuffer->proj = camera.projection();
    matrixBuffer.update();
    camera.projectionDirty = false;
  }

  if (camera.lightDirty) {
    lightBuffer[0].direction = camera.lightPosition();
    camera.lightDirty = false;
  }
}

void Renderer::updateModels() {
  if (rotateModel) {
    grieghallen.modelTransform = glm::rotate(
      grieghallen.modelTransform, 0.01f, glm::vec3(0, 1, 0));
    bigSuzy.modelTransform = glm::rotate(
      bigSuzy.modelTransform, 0.01f, glm::vec3(0, 1, 0));
    terrain.modelTransform = glm::rotate(
      terrain.modelTransform, 0.01f, glm::vec3(0, 1, 0));
  }

  if (moveLights) {
    {
      auto &position = lightBuffer[1].position;
      position = { cos(_lightAngle), 0.0f, sin(_lightAngle) };
      position *= 70.0f + 25.0f * sin(_lightAngle);
      lightBuffer[1].direction = glm::normalize(-position);
      lightBuffer[1].direction.y -= _lightTilt;
      suzanne1.setPosition(position / 20.0f);
    }

    {
      auto &position = lightBuffer[2].position;
      position = { cos(-_lightAngle), 0.0f, sin(-_lightAngle) };
      position *= 50.0f;
      suzanne2.setPosition(position / 20.0f);
    }

    _lightAngle += 0.005f;
  }

  {
    auto &light = lightBuffer[1];
    if (light.type == 3) {
      light.direction = glm::normalize(-light.position);
      light.direction.y += _lightTilt;
    }
  }

  {
    auto &light = lightBuffer[2];
    if (light.type == 3) {
      light.direction = glm::normalize(-light.position);
      light.direction.y += _lightTilt;
    }
  }

  _lightTilt += _tiltFactor;
  if (_lightTilt > 1.0f || _lightTilt < -1.0f) {
    _tiltFactor *= -1.0f;
  }

  lightBuffer.update();
}

void Renderer::setAllShaders(std::shared_ptr<Shader> shader) {
  grieghallen.setShader(shader);
  suzanne1.setShader(shader);
  suzanne2.setShader(shader);
  bigSuzy.setShader(shader);
  terrain.setShader(shader);
}

void Renderer::drawAll() {
  switch (currentModel) {
    case 0:
      grieghallen.draw();
      break;

    case 1:
      if (waterized) {
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

  if (lightBuffer[1].type != 0) {
    suzanne1.draw();
  }
  if (lightBuffer[2].type != 0) {
    suzanne2.draw();
  }
}

void Renderer::setModelRotation(bool rotate) {
  rotateModel = rotate;
}

void Renderer::setModel(int model) {
  currentModel = model % 3;
}

void Renderer::rotateLights(bool move) {
  moveLights = move;
}

void Renderer::setShader(int shader) {
  shader %= 3;
  switch (shader) {
  case 0:
    mPostprocessShader = nullptr;
    break;

  case 1:
    mPostprocessShader = toonShader;
    break;

  case 2:
    mPostprocessShader = depthShader;
    break;

  default:
    break;
  }
}

void Renderer::showPanel(int light) {
  if (dlgLight == nullptr) {
    dlgLight = new View::LightDialog(this);
  }

  static_cast<View::LightDialog*>(dlgLight)->show(lightBuffer[light], light);
}

void Renderer::setAmbient(int level) {
  ambientLevel = level / 100.0f;
}

void Renderer::initializeGL() {
  initializeOpenGLFunctions();

#define glReport(x) println(#x ": {}", reinterpret_cast<const char*>(glGetString(x)))
  glReport(GL_VENDOR);
  glReport(GL_RENDERER);
  glReport(GL_VERSION);
  glReport(GL_SHADING_LANGUAGE_VERSION);
#undef glReport

  generateFrameBuffer();

  water->load("water.jpg", 16);

  gridShader->load("grid", ShaderType::object);
  gridShader->bindBuffer(matrixBuffer);

  basicShader->load("basic", ShaderType::object);
  basicShader->bindBuffer(matrixBuffer);
  basicShader->bindBuffer(lightBuffer);
  basicShader->uniform("uTexture") = Sampler2D(TEXTURE_LOCATION);
  basicShader->uniform("uBump") = Sampler2D(BUMP_LOCATION);

  cubemap.load();
  cubemap.bindBuffer(matrixBuffer);

  toonShader->load("toon", ShaderType::postprocess);
  toonShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  toonShader->uniform("uNormalbuffer") = Sampler2D(NORMALBUFFER_LOCATION);
  toonShader->uniform("uDepthbuffer") = Sampler2D(DEPTHBUFFER_LOCATION);
  toonShader->uniform("uScreenSize") = glm::vec2(width(), height());

  depthShader->load("depth", ShaderType::postprocess);
  depthShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  depthShader->uniform("uDepth") = Sampler2D(LINEARDEPTHBUFFER_LOCATION);
  depthShader->uniform("uScreenSize") = glm::vec2(width(), height());

  grieghallen.load("grieghallen.obj");
  grieghallen.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  suzanne1.load("suzanne.obj");
  suzanne1.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  suzanne2.load("suzanne.obj");
  suzanne2.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  bigSuzy.load("suzanne.obj");

  //terrain.load("suzanne.obj");
  terrain.load("bergen_1024x918.bin");
  //terrain.load("bergen_2048x1836.bin");
  //terrain.load("bergen_3072x2754.bin");
  terrain.modelTransform = glm::scale(Mat4(), Vec3(4.0f, 4.0f, 4.0f));

  bergen->load("bergen_terrain_texture.png");
  terrain.setMaterial(bergen, { 0.0f, 0.0f, 0.0f });

  bump->load("Rock.jpg");
  bigSuzy.setBump(bump);

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
  glGenVertexArrays(1, &gridVao);
  glBindVertexArray(gridVao);

  glGenBuffers(1, &gridVbo);
  glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(gridQuad),
               &gridQuad[0],
               GL_STATIC_DRAW);

  glClearColor(0, 0, 0, 1);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
  glActiveTexture(GL_TEXTURE0 + NORMALBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, normalBufferTexture);
  glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
  glActiveTexture(GL_TEXTURE0 + LINEARDEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, linearDepthBufferTexture);

  timer.start();
}

void Renderer::resizeGL(int width, int height) {
  camera.resize(width, height);

  glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  glActiveTexture(GL_TEXTURE0 + NORMALBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, normalBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  glActiveTexture(GL_TEXTURE0 + LINEARDEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, linearDepthBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_R8, GL_FLOAT, 0);

  glViewport(0, 0, width, height);

  toonShader->uniform("uScreenSize") = glm::vec2(width, height);
  depthShader->uniform("uScreenSize") = glm::vec2(width, height);
}

void Renderer::paintGL() {
  checkAndLoadUniforms();
  updateModels();

  if (mPostprocessShader) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  }

  /* Draw grid before doing anything else */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glBindVertexArray(gridVao);
  glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
  gridShader->use();
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat) * 3, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableVertexAttribArray(0);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  setAllShaders(basicShader);
  basicShader->uniform("uAmbientLight") = glm::vec3(ambientLevel);

  drawAll();

  cubemap.draw();

  if (mPostprocessShader) {
    QOpenGLFramebufferObject::bindDefault();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mPostprocessShader->use();

    // The triangles are defined in the postprocessor's vertex shader
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  glUseProgram(0);

  if (timer.elapsed() >= 1000) {
    fpsText = fmt::format("FPS: {}", fpsCount / 2);
    fpsCount = 0;
    timer.restart();
    lblFPS->setText(fpsText.c_str());
  }
  fpsCount++;

  update();
}

void Renderer::mousePressEvent(QMouseEvent *evt) {
  camera.mousePressed(evt);
}

void Renderer::mouseMoveEvent(QMouseEvent *evt) {
  camera.mouseMoved(evt);
}

void Renderer::wheelEvent(QWheelEvent *evt) {
  camera.wheelMoved(evt);
}

void Renderer::keyPressEvent(QKeyEvent *evt) {
  camera.keyPressed(evt);
}

void Renderer::keyReleaseEvent(QKeyEvent *evt) {
  if (evt->key() == Qt::Key_Escape) {
    static_cast<QWidget*>(parent())->close();
  } else {
    camera.keyReleased(evt);
  }
}

void Renderer::generateFrameBuffer() {
  // Color attachment
  glGenTextures(1, &frameBufferTexture);
  glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // Normal attachment
  glGenTextures(1, &normalBufferTexture);
  glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, normalBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // Depth attachment
  glGenTextures(1, &depthBufferTexture);
  glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, depthBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width(), height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  // Linear depth attachment
  glGenTextures(1, &linearDepthBufferTexture);
  glActiveTexture(GL_TEXTURE0 + LINEARDEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, linearDepthBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width(), height(), 0, GL_RED, GL_FLOAT, 0);

  // Actual frame buffer
  glGenFramebuffers(1, &frameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalBufferTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, linearDepthBufferTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

  GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, attachments);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

