#include "MainWindow.hh"

#include <QMenu>
#include <QMenuBar>
#include <memory>
#include <QSignalMapper>

namespace Ui {
  MainWindow::MainWindow() {

    QMenuBar *mnbMenu = new QMenuBar();

    // File menu
    {
      QMenu *mnuFile = mnbMenu->addMenu("&File");
      QAction *actExit = new QAction("Exit", mnuFile);
      mnuFile->addAction(actExit);
      connect(actExit, &QAction::triggered, this, &QMainWindow::close);
    }
    
    // Model menu
    {
      QMenu *mnuModel = mnbMenu->addMenu("&Model");

      QAction *actGrieg = new QAction("Grieghallen", mnuModel);
      QAction *actSuzy = new QAction("Big Suzy", mnuModel);
      QAction *actTerrain = new QAction("Terrain", mnuModel);
      
      mnuModel->addAction(actGrieg);
      mnuModel->addAction(actSuzy);
      mnuModel->addAction(actTerrain);

      QActionGroup *grpModel = new QActionGroup(mnuModel);
      grpModel->addAction(actGrieg);
      grpModel->addAction(actSuzy);
      grpModel->addAction(actTerrain);
      actGrieg->setChecked(true);

      QSignalMapper *mapper = new QSignalMapper(this);
      mapper->setMapping(actGrieg, 0);
      mapper->setMapping(actSuzy, 1);
      mapper->setMapping(actTerrain, 2);

      mnuModel->addSeparator();
      
      QAction *actRotate = new QAction("Rotate", mnuModel);
      actRotate->setCheckable(true);
      actRotate->setChecked(false);
      mnuModel->addAction(actRotate);

      connect(actGrieg, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actSuzy, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actTerrain, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              this, SLOT(setModel(int)));
      connect(actRotate, SIGNAL(triggered(bool)),
              this, SLOT(setModelRotation(bool)));
    }

    QMenu *mnuCamera = mnbMenu->addMenu("&Camera");
    QMenu *mnuShader = mnbMenu->addMenu("&Shader");
    QMenu *mnuLight = mnbMenu->addMenu("&Light");
    setMenuBar(mnbMenu);

    //setAttribute(Qt::WA_DeleteOnClose);
    //setAttribute(Qt::WA_QuitOnClose);
  }

  void MainWindow::attachRenderer(OpenGLWidget * openGL) {
    setCentralWidget(openGL);
  }
}
