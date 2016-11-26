#include "MainWindow.hh"

#include <QMenu>
#include <QMenuBar>
#include <memory>

namespace Ui {
  MainWindow::MainWindow() {
    QMenuBar *mnbMenu = new QMenuBar();
    QMenu *mnuFile = mnbMenu->addMenu("&File");
    QMenu *mnuCamera = mnbMenu->addMenu("&Camera");
    QMenu *mnuShader = mnbMenu->addMenu("&Shader");
    QMenu *mnuLight = mnbMenu->addMenu("&Light");
    setMenuBar(mnbMenu);

    setAttribute(Qt::WA_QuitOnClose);
  }

  void MainWindow::attachRenderer(OpenGLWidget * openGL) {
    setCentralWidget(openGL);
  }
}
