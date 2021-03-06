#include "Renderer.hh"

#include <QOpenGLFramebufferObject>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QApplication>

#include <thread>

#include "LightDialog.hh"

namespace {

  constexpr int TEXTURE_LOCATION = 0;
  constexpr int BUMP_LOCATION = 2;
  constexpr int FRAMEBUFFER_LOCATION = 10;
  constexpr int NORMALBUFFER_LOCATION = 11;
  constexpr int DEPTHBUFFER_LOCATION = 12;
  //constexpr int LINEARDEPTHBUFFER_LOCATION = 13;
  constexpr int STENCILBUFFER_LOCATION = 14;

  bool moveLights = true;
  float ambientLevel = 0.4f;
  bool rotateModel = false;
  Renderer::Model currentModel = Renderer::BERGEN_LOW;
  bool currentWaterized = false;
  bool showCubemap = true;
  bool loading = false;

  GLuint gridVbo = 0;
  GLuint gridVao = 0;

  GLuint frameBuffer;
  GLuint frameBufferTexture;
  GLuint normalBufferTexture;
  GLuint depthBufferTexture;
  //GLuint linearDepthBufferTexture;

  float _lightAngle{};
  float _lightTilt{};
  float _tiltFactor{ 0.01f };

  QElapsedTimer timer;
  std::string fpsText = "FPS: 0";
  uint32_t fpsCount = 0;
}

Renderer::Renderer(QWidget *parent) :
  QOpenGLWidget(parent),
  camera(this) {
  basicShader = std::make_shared<Shader>();
  ambientShader = std::make_shared<Shader>();
  normalsShader = std::make_shared<Shader>();
  heightShader = std::make_shared<Shader>();
  gridShader = std::make_shared<Shader>();
  lineShader = std::make_shared<Shader>();

  toonShader = std::make_shared<Shader>();
  depthShader = std::make_shared<Shader>();
  fogShader = std::make_shared<Shader>();
  identityShader = std::make_shared<Shader>();

  water = std::make_shared<Texture>();
  bump = std::make_shared<Texture>();
  bergen = std::make_shared<Texture>();
}

void Renderer::checkAndLoadUniforms() {
  if (camera.viewDirty) {
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

  cubemap.shader.uniform("uPV") = camera.skyboxPV();
}

void Renderer::updateModels() {
  if (rotateModel) {
    bigSuzy.modelTransform = glm::rotate(
      bigSuzy.modelTransform, 0.01f, glm::vec3(0, 1, 0));
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
  if (shader == heightShader) {
    grieghallen.setShader(basicShader);
    suzanne1.setShader(basicShader);
    suzanne2.setShader(basicShader);
    bigSuzy.setShader(basicShader);
  } else {
    grieghallen.setShader(shader);
    suzanne1.setShader(shader);
    suzanne2.setShader(shader);
    bigSuzy.setShader(shader);
  }

  //if (shader == basicShader) {
  //  terrain.setShader(ambientShader);
  //} else {
    terrain.setShader(shader);
  //}
}

void Renderer::drawAll() {
  gl->glEnable(GL_STENCIL_TEST);
  gl->glStencilFunc(GL_ALWAYS, 1, 0xff);
  gl->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  gl->glStencilMask(0xff);
  gl->glClear(GL_STENCIL_BUFFER_BIT);

  switch (currentModel) {
    case BERGEN_LOW:
    case BERGEN_MID:
    case BERGEN_HI:
    default:
      terrain.draw();
      grieghallen.draw();
      break;

    case SUZY_BUMP:
    case SUZY_WATER:
      bigSuzy.draw();
      break;
  }

  if (lightBuffer[1].type != 0) {
    suzanne1.draw();
  }
  if (lightBuffer[2].type != 0) {
    suzanne2.draw();
  }

  gl->glDisable(GL_STENCIL_TEST);
}

void Renderer::setModelRotation(bool rotate) {
  rotateModel = rotate;
}

void Renderer::setModel(Renderer::Model model) {
  if (currentModel == model) {
    return;
  }

  currentModel = model;
  loading = true;
  repaint();

  switch (model) {
    case BERGEN_LOW:
    default:
    {
      terrain.load("bergen_1024x918.bin");
      break;
    }
    case BERGEN_MID:
    {
      terrain.load("bergen_2048x1836.bin");
      break;
    }
    case BERGEN_HI:
    {
      terrain.load("bergen_3072x2754.bin");
      break;
    }
    case SUZY_BUMP:
      bigSuzy.setBump(bump);
      break;
    case SUZY_WATER:
      bigSuzy.setMaterial(water);
      break;
  }

  loading = false;
  repaint();
}

void Renderer::rotateLights(bool move) {
  moveLights = move;
}

void Renderer::setShader(int shader) {
  shader %= 7;

  if (shader == 4) {
    grieghallen.enableTexture = false;
    suzanne1.enableTexture = false;
    suzanne2.enableTexture = false;
    bigSuzy.enableTexture = false;
    terrain.enableTexture = false;
    showCubemap = false;
  } else {
    grieghallen.enableTexture = true;
    suzanne1.enableTexture = true;
    suzanne2.enableTexture = true;
    bigSuzy.enableTexture = true;
    terrain.enableTexture = true;
    showCubemap = shader != 6;
  }

  switch (shader) {
    case 4:
      mPostprocessShader = toonShader;
      break;

    case 5:
      mPostprocessShader = depthShader;
      break;

    case 6:
      mPostprocessShader = fogShader;
      break;
      
    default:
      mPostprocessShader = nullptr;
      break;
  }

  switch (shader) {
    case 1:
      mObjectShader = ambientShader;
      break;

    case 2:
      mObjectShader = normalsShader;
      break;

    case 3:
      mObjectShader = heightShader;
      break;

    default:
      mObjectShader = basicShader;
      break;
  }

  setAllShaders(mObjectShader);
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

  lineShader->load("lines", ShaderType::object);
  lineShader->bindBuffer(matrixBuffer);

  basicShader->load("basic", ShaderType::object);
  basicShader->bindBuffer(matrixBuffer);
  basicShader->bindBuffer(lightBuffer);
  basicShader->uniform("uTexture") = Sampler2D(TEXTURE_LOCATION);
  basicShader->uniform("uBump") = Sampler2D(BUMP_LOCATION);

  ambientShader->load("ambient", ShaderType::object);
  ambientShader->bindBuffer(matrixBuffer);
  ambientShader->bindBuffer(lightBuffer);
  ambientShader->uniform("uTexture") = Sampler2D(TEXTURE_LOCATION);
  ambientShader->uniform("uBump") = Sampler2D(BUMP_LOCATION);

  normalsShader->load("normals", ShaderType::object);
  normalsShader->bindBuffer(matrixBuffer);
  normalsShader->bindBuffer(lightBuffer);
  normalsShader->uniform("uTexture") = Sampler2D(TEXTURE_LOCATION);
  normalsShader->uniform("uBump") = Sampler2D(BUMP_LOCATION);

  heightShader->load("height", ShaderType::object);
  heightShader->bindBuffer(matrixBuffer);
  heightShader->bindBuffer(lightBuffer);
  heightShader->uniform("uTexture") = Sampler2D(TEXTURE_LOCATION);
  heightShader->uniform("uBump") = Sampler2D(BUMP_LOCATION);

  cubemap.load();

  toonShader->load("toon", ShaderType::postprocess);
  toonShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  toonShader->uniform("uNormalbuffer") = Sampler2D(NORMALBUFFER_LOCATION);
  toonShader->uniform("uDepthbuffer") = Sampler2D(DEPTHBUFFER_LOCATION);
  //toonShader->uniform("uDepth") = Sampler2D(LINEARDEPTHBUFFER_LOCATION);
  toonShader->uniform("uScreenSize") = glm::vec2(width(), height());

  depthShader->load("depth", ShaderType::postprocess);
  depthShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  depthShader->uniform("uDepthbuffer") = Sampler2D(DEPTHBUFFER_LOCATION);
  //depthShader->uniform("uDepth") = Sampler2D(LINEARDEPTHBUFFER_LOCATION);
  depthShader->uniform("uScreenSize") = glm::vec2(width(), height());

  fogShader->load("fog", ShaderType::postprocess);
  fogShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  fogShader->uniform("uDepthbuffer") = Sampler2D(DEPTHBUFFER_LOCATION);
  //fogShader->uniform("uDepth") = Sampler2D(LINEARDEPTHBUFFER_LOCATION);
  fogShader->uniform("uScreenSize") = glm::vec2(width(), height());

  identityShader->load("identity", ShaderType::postprocess);
  identityShader->uniform("uFramebuffer") = Sampler2D(FRAMEBUFFER_LOCATION);
  identityShader->uniform("uScreenSize") = glm::vec2(width(), height());

  grieghallen.load("grieghallen.obj");
  grieghallen.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  suzanne1.load("suzanne.obj");
  suzanne1.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  suzanne2.load("suzanne.obj");
  suzanne2.modelTransform = glm::scale(Mat4(), Vec3(0.02f, 0.02f, 0.02f));

  bigSuzy.load("suzanne.obj");

  terrain.load("bergen_1024x918.bin");

  constexpr float ratio = 120.0f;
  terrain.modelTransform = glm::translate(terrain.modelTransform, { 0.0f, -0.145f, 0.0f });
  terrain.modelTransform = glm::scale(terrain.modelTransform, Vec3(ratio, ratio, ratio));
  terrain.modelTransform = glm::translate(terrain.modelTransform, { -0.202f, 0.0f, -0.1675f });
  terrain.modelTransform = glm::rotate(terrain.modelTransform, 3.5f, { 0.0f, 1.0f, 0.0f });

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
  //glActiveTexture(GL_TEXTURE0 + LINEARDEPTHBUFFER_LOCATION);
  //glBindTexture(GL_TEXTURE_2D, linearDepthBufferTexture);

  setShader(0); // set to 'basic' shader

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

  //glActiveTexture(GL_TEXTURE0 + LINEARDEPTHBUFFER_LOCATION);
  //glBindTexture(GL_TEXTURE_2D, linearDepthBufferTexture);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, 0);

  glViewport(0, 0, width, height);

  toonShader->uniform("uScreenSize") = glm::vec2(width, height);
  depthShader->uniform("uScreenSize") = glm::vec2(width, height);
  fogShader->uniform("uScreenSize") = glm::vec2(width, height);
}

void Renderer::paintGL() {
  if (loading) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return;
  }
  camera.update();

  checkAndLoadUniforms();
  updateModels();

  if (mPostprocessShader) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  }

  /* Draw grid before doing anything else */
  ambientShader->uniform("uAmbientLight") = glm::vec3(ambientLevel);
  basicShader->uniform("uAmbientLight") = glm::vec3(ambientLevel);
  heightShader->uniform("uAmbientLight") = glm::vec3(ambientLevel);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  drawAll();

  if (showCubemap)
    cubemap.draw();

  if (mPostprocessShader) {
    QOpenGLFramebufferObject::bindDefault();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
    glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(), GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilMask(0);

    mPostprocessShader->use();

    // The triangles are defined in the postprocessor's vertex shader
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    identityShader->use();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_STENCIL_TEST);
  }

  QOpenGLFramebufferObject::bindDefault();
  glUseProgram(0);

  if (timer.elapsed() >= 1000) {
    fpsText = fmt::format("FPS: {}", fpsCount);
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

void Renderer::mouseReleaseEvent(QMouseEvent *evt) {
  camera.mouseReleased(evt);
}

void Renderer::mouseMoveEvent(QMouseEvent *evt) {
  camera.mouseMoved(evt);
}

void Renderer::wheelEvent(QWheelEvent *evt) {
  camera.wheelMoved(evt);
}

void Renderer::generateFrameBuffer() {
  // Color attachment
  glGenTextures(1, &frameBufferTexture);
  glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // Normal attachment
  glGenTextures(1, &normalBufferTexture);
  glActiveTexture(GL_TEXTURE0 + FRAMEBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, normalBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // Depth attachment
  glGenTextures(1, &depthBufferTexture);
  glActiveTexture(GL_TEXTURE0 + DEPTHBUFFER_LOCATION);
  glBindTexture(GL_TEXTURE_2D, depthBufferTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width(), height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

  // Linear depth attachment
  //glGenTextures(1, &linearDepthBufferTexture);
  //glActiveTexture(GL_TEXTURE0 + LINEARDEPTHBUFFER_LOCATION);
  //glBindTexture(GL_TEXTURE_2D, linearDepthBufferTexture);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  //glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width(), height(), 0, GL_RED, GL_FLOAT, 0);

  // Actual frame buffer
  glGenFramebuffers(1, &frameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalBufferTexture, 0);
  //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, linearDepthBufferTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

  GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachments);

  QOpenGLFramebufferObject::bindDefault();
}

