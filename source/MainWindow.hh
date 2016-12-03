#ifndef __INF251_QT_MAINWINDOW__2733556
#define __INF251_QT_MAINWINDOW__2733556

#include <QMainWindow>
#include <QCloseEvent>
#include <QSlider>
#include "Renderer.hh"

namespace View {
  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();
    ~MainWindow() = default;

    void attachRenderer(Renderer * renderer);

  private:
    Renderer * mRenderer = nullptr;
    Camera * camera = nullptr;
    QAction * actPerspective = nullptr;

    QLabel * lblFPS = nullptr;
    QLabel * lblPosition = nullptr;
    QSlider * sldAmbient = nullptr;

    bool ortho = false;
    void buildMenu();

    private slots:
    void togglePerspective();
    void resetCamera();
    void showHelp();
    void toggleFullscreen();

  };
}

#endif //__INF251_QT_MAINWINDOW__2733556
