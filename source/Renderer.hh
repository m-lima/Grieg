#ifndef __INF251_RENDERER__48721384
#define __INF251_RENDERER__48721384

#include <memory>
#include <QStatusBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <glm/gtc/matrix_transform.hpp>

#include "Object.hh"
#include "Shader.hh"
#include "Texture.hh"
#include "ShaderStorage.hh"

class Renderer : public QOpenGLWidget, public QOpenGLFunctions_4_3_Core {
  Q_OBJECT

public:

  Renderer(QWidget *parent = 0);
  ~Renderer() = default;

  void checkAndLoadUniforms();

  public slots:
  void setModelRotation(bool rotate);
  void setModel(int model);
  void cycleLights();
  void setSpotlight(bool spotlight);
  void setSun(bool sun);
  void setLightMovement(bool move);
  void setShader(int shader);
  void setPosition(int position);
  void togglePerspective();
  void resetCamera();

  void setStatusBar(QStatusBar * status) {
    statusBar = status;
  }

protected:
  void initializeGL() Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;
  void resizeGL(int width, int height) Q_DECL_OVERRIDE;

private:

  struct MatrixBlock {
    static constexpr auto name = "MatrixBlock";
    static constexpr auto binding = 0;

    glm::mat4 proj;
    glm::mat4 view;
  };

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

  void generateFrameBuffer();
  void setAllShaders(std::shared_ptr<Shader> shader);
  void drawAll();

  std::shared_ptr<Shader> basicShader;
  std::shared_ptr<Shader> toonShader;
  std::shared_ptr<Shader> depthShader;
  std::shared_ptr<Shader> gridShader;

  std::shared_ptr<Texture> water;
  std::shared_ptr<Texture> bump;
  std::shared_ptr<Texture> bergen;

  Object grieghallen;
  Object suzanne1;
  Object suzanne2;
  Object bigSuzy;
  Object terrain;

  ShaderStorage<MatrixBlock> matrixBuffer;
  ShaderStorage<LightBlock[], 12> lightBuffer;

  QStatusBar *statusBar = nullptr;

};

#endif //__INF251_RENDERER__48721384
