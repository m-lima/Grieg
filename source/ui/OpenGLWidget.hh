#ifndef __INF251_QT_OPENGLWIDGET__2286954
#define __INF251_QT_OPENGLWIDGET__2286954

#include "../infdef.hh"
#include <QOpenGLWidget>

namespace Ui {
  class OpenGLWidget : public QOpenGLWidget , protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT

  public:
    OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget() = default;

    void setInitGL(void(*initGL)(int, int)) {
      _initGL = initGL;
    }

    void setDrawGL(void(*drawGL)()) {
      _drawGL = drawGL;
    }
    void setResizeGL(void(*resizeGL)(int, int)) {
      _resizeGL = resizeGL;
    }

  protected:
    void initializeGL() Q_DECL_OVERRIDE {
      _initGL(width(), height());
    }
    void paintGL() Q_DECL_OVERRIDE {
      _drawGL();
    }
    void resizeGL(int width, int height) Q_DECL_OVERRIDE {
      _resizeGL(width, height);
    }
    //void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

  private:
    void(*_initGL) (int, int);
    void(*_drawGL) ();
    void(*_resizeGL) (int, int);

  };
}

#endif //__INF251_QT_OPENGLWIDGET__2286954