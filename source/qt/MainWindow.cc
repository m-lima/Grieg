#include "MainWindow.hh"

#include "OpenGLWidget.hh"

#include <QMenu>
#include <QMenuBar>
#include <memory>

namespace uiQT {
  MainWindow::MainWindow() {
    QMenuBar *mnbMenu = new QMenuBar();
    QMenu *mnuFile = mnbMenu->addMenu("&File");
    QMenu *mnuCamera = mnbMenu->addMenu("&Camera");
    QMenu *mnuShader = mnbMenu->addMenu("&Shader");
    QMenu *mnuLight = mnbMenu->addMenu("&Light");
    setMenuBar(mnbMenu);

    setAttribute(Qt::WA_QuitOnClose);
    setCentralWidget(new OpenGLWidget(this));
  }
}
