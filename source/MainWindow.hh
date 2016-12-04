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

  protected:
    void keyPressEvent(QKeyEvent *evt) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *evt) Q_DECL_OVERRIDE;
    bool event(QEvent *evt) Q_DECL_OVERRIDE;

  private:
    Renderer * mRenderer = nullptr;
    Camera * camera = nullptr;
    QAction * actPerspective = nullptr;
    QAction * actTrack = nullptr;

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
