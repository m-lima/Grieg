#ifndef __INF251_QT_MAINWINDOW__2733556
#define __INF251_QT_MAINWINDOW__2733556

#include "../infdef.hh"

#include <QMainWindow>
#include "OpenGLWidget.hh"

namespace Ui {
  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();
    ~MainWindow() = default;

    void attachRenderer(OpenGLWidget * openGL);

    protected slots:

    void setModelRotation(bool rotate) {
      _setModelRotation(rotate);
    }

    void setModel(int model) {
      _setModel(model);
    }

    void cycleLights() {
      _cycleLights();
    }

    void setSpotlight(bool spotlight) {
      _setSpotlight(spotlight);
    }

    void setSun(bool sun) {
      _setSun(sun);
    }

    void setLightMovement(bool move) {
      _setLightMovement(move);
    }

    void setShader(int shader) {
      _setShader(shader);
    }

  private:
    void(*_setModelRotation) (bool);
    void(*_setModel) (int);
    void(*_cycleLights) ();
    void(*_setSpotlight) (bool);
    void(*_setSun) (bool);
    void(*_setLightMovement) (bool);
    void(*_setShader) (int);
  };
}

#endif //__INF251_QT_MAINWINDOW__2733556
