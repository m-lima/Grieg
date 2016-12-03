#ifndef __INF251_RENDERER__48721384
#define __INF251_RENDERER__48721384

#include <QLabel>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <glm/gtc/matrix_transform.hpp>

#include "Object.hh"
#include "Shader.hh"
#include "Texture.hh"
#include "ShaderStorage.hh"
#include "Trackball.hh"
#include "Cubemap.hh"

class Renderer : public QOpenGLWidget, public QOpenGLFunctions_4_3_Core {
  Q_OBJECT

public:
  Trackball trackball;

  Renderer(QWidget *parent = 0);
  ~Renderer() = default;

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

  public slots:
  void setModelRotation(bool rotate);
  void setModel(int model);
  void rotateLights(bool move);
  void setShader(int shader);
  void showPanel(int light);
  void setAmbient(int level);

  void setFPS(QLabel * labelFPS) {
    lblFPS = labelFPS;
  }

  void setPosition(QLabel * labelPosition) {
    lblPosition = labelPosition;
  }

protected:
  void initializeGL() Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;
  void resizeGL(int width, int height) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *evt) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *evt) Q_DECL_OVERRIDE;
  void keyReleaseEvent(QKeyEvent *evt) Q_DECL_OVERRIDE;
  void wheelEvent(QWheelEvent *evt) Q_DECL_OVERRIDE;

private:

  struct MatrixBlock {
    static constexpr auto name = "MatrixBlock";
    static constexpr auto binding = 0;

    glm::mat4 proj;
    glm::mat4 view;
  };

  void checkAndLoadUniforms();
  void updateModels();
  void generateFrameBuffer();
  void setAllShaders(std::shared_ptr<Shader> shader);
  void drawAll();

  std::shared_ptr<Shader> basicShader;
  std::shared_ptr<Shader> toonShader;
  std::shared_ptr<Shader> depthShader;
  std::shared_ptr<Shader> gridShader;

  std::shared_ptr<Shader> mPostprocessShader;

  std::shared_ptr<Texture> water;
  std::shared_ptr<Texture> bump;
  std::shared_ptr<Texture> bergen;

  Object grieghallen;
  Object suzanne1;
  Object suzanne2;
  Object bigSuzy;
  Object terrain;

  Cubemap cubemap;

  ShaderStorage<MatrixBlock> matrixBuffer;
  ShaderStorage<LightBlock[], 12> lightBuffer;

  QLabel * lblFPS = nullptr;
  QLabel * lblPosition = nullptr;
  QDialog * dlgLight = nullptr;
};

#endif //__INF251_RENDERER__48721384
