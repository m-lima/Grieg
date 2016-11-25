#ifndef __INF251_QT_OPENGLWIDGET__2286954
#define __INF251_QT_OPENGLWIDGET__2286954

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

namespace uiQT {
  class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

  public:
    OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget() = default;

  };
}

#endif //__INF251_QT_OPENGLWIDGET__2286954