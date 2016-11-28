#ifndef __INF251_QT_MAINWINDOW__2733556
#define __INF251_QT_MAINWINDOW__2733556

#include <QMainWindow>
#include <QCloseEvent>
#include "Renderer.hh"

namespace Ui {
  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();
    ~MainWindow() = default;

    void attachRenderer(Renderer * renderer);

  private:
    Renderer * mRenderer = nullptr;
    Trackball * trackball = nullptr;
    QAction * actPerspective = nullptr;
    QLabel * lblFPS = nullptr;
    bool ortho = false;
    bool full = false;
    void buildMenu();

    private slots:
    void togglePerspective();
    void resetCamera();
    void showHelp();
    void toggleFullscreen();

  };
}

#endif //__INF251_QT_MAINWINDOW__2733556
