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
  };
}

#endif //__INF251_QT_MAINWINDOW__2733556
