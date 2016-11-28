#include "MainWindow.hh"
#include "HelpDialog.hh"

#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <memory>
#include <QSignalMapper>
#include <QShortcut>

namespace Ui {
  MainWindow::MainWindow() {
    QStatusBar *status = new QStatusBar(this);
    lblFPS = new QLabel(status);
    status->addWidget(lblFPS);
    setStatusBar(status);
  }

  void MainWindow::attachRenderer(Renderer * renderer) {
    if (mRenderer == nullptr) {
      mRenderer = renderer;
      renderer->setFPS(lblFPS);
      trackball = &(mRenderer->trackball);
      buildMenu();
      setCentralWidget(renderer);
      resetCamera();
    }
  }

  void MainWindow::buildMenu() {
    QMenuBar *mnbMenu = new QMenuBar();
    QMenu *menu;
    QActionGroup *group;
    QSignalMapper *mapper;

    // File menu
    {
      menu = mnbMenu->addMenu("&File");

      QAction *actFull = new QAction("Fullscreen", menu);
      QAction *actExit = new QAction("Exit", menu);

      actFull->setIcon(QIcon(":images/full.png"));
      actExit->setIcon(QIcon(":images/exit.png"));

      actFull->setShortcutContext(Qt::ApplicationShortcut);
      actExit->setShortcutContext(Qt::ApplicationShortcut);
      
      actFull->setShortcut(QKeySequence(Qt::Key_F));
      actExit->setShortcut(QKeySequence(Qt::Key_Escape));
      
      menu->addAction(actFull);
      menu->addSeparator();
      menu->addAction(actExit);

      connect(actFull, &QAction::triggered, this, &MainWindow::toggleFullscreen);
      connect(actExit, &QAction::triggered, this, &QMainWindow::close);
    }

    // Model menu
    {
      menu = mnbMenu->addMenu("&Model");

      QAction *actGrieg = new QAction("Grieghallen", menu);
      QAction *actSuzy = new QAction("Big Suzy", menu);
      QAction *actTerrain = new QAction("Terrain", menu);
      QAction *actRotate = new QAction("Rotate", menu);

      actGrieg->setCheckable(true);
      actSuzy->setCheckable(true);
      actTerrain->setCheckable(true);

      group = new QActionGroup(menu);
      group->addAction(actGrieg);
      group->addAction(actSuzy);
      group->addAction(actTerrain);
      actGrieg->setChecked(true);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actGrieg, 0);
      mapper->setMapping(actSuzy, 1);
      mapper->setMapping(actTerrain, 2);

      actRotate->setCheckable(true);
      actRotate->setChecked(false);
      actRotate->setShortcutContext(Qt::ApplicationShortcut);
      actRotate->setShortcut(QKeySequence(Qt::Key_R));

      menu->addAction(actGrieg);
      menu->addAction(actSuzy);
      menu->addAction(actTerrain);
      menu->addSeparator();
      menu->addAction(actRotate);

      connect(actGrieg, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actSuzy, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actTerrain, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              mRenderer, SLOT(setModel(int)));
      connect(actRotate, SIGNAL(triggered(bool)),
              mRenderer, SLOT(setModelRotation(bool)));
    }

    // Shader menu
    {
      menu = mnbMenu->addMenu("&Shader");

      QAction *actBasic = new QAction("Basic", menu);
      QAction *actToon = new QAction("Toon", menu);
      QAction *actTilt = new QAction("Tilt-shift", menu);

      actBasic->setCheckable(true);
      actToon->setCheckable(true);
      actTilt->setCheckable(true);

      group = new QActionGroup(menu);
      group->addAction(actBasic);
      group->addAction(actToon);
      group->addAction(actTilt);
      actBasic->setChecked(true);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actBasic, 0);
      mapper->setMapping(actToon, 1);
      mapper->setMapping(actTilt, 2);

      menu->addAction(actBasic);
      menu->addAction(actToon);
      menu->addAction(actTilt);

      connect(actBasic, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actToon, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actTilt, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              mRenderer, SLOT(setShader(int)));
    }

    // Camera menu
    {
      menu = mnbMenu->addMenu("&Camera");

      QAction *actTop = new QAction("Top", menu);
      QAction *actBottom = new QAction("Bottom", menu);
      QAction *actRight = new QAction("Right", menu);
      QAction *actLeft = new QAction("Left", menu);
      QAction *actFront = new QAction("Front", menu);
      QAction *actBack = new QAction("Back", menu);
      actPerspective = new QAction("Orthographic", menu);
      QAction *actReset = new QAction("Reset", menu);

      actPerspective->setShortcutContext(Qt::ApplicationShortcut);
      actPerspective->setShortcut(QKeySequence(Qt::Key_Space));

      actTop->setIcon(QIcon(":images/top.png"));
      actBottom->setIcon(QIcon(":images/bottom.png"));
      actRight->setIcon(QIcon(":images/right.png"));
      actLeft->setIcon(QIcon(":images/left.png"));
      actFront->setIcon(QIcon(":images/front.png"));
      actBack->setIcon(QIcon(":images/back.png"));
      actPerspective->setIcon(QIcon(":images/ortho.png"));
      actReset->setIcon(QIcon(":images/reset.png"));

      mapper = new QSignalMapper(this);
      mapper->setMapping(actTop, 0);
      mapper->setMapping(actBottom, 1);
      mapper->setMapping(actRight, 2);
      mapper->setMapping(actLeft, 3);
      mapper->setMapping(actFront, 4);
      mapper->setMapping(actBack, 5);

      menu->addAction(actTop);
      menu->addAction(actBottom);
      menu->addAction(actRight);
      menu->addAction(actLeft);
      menu->addAction(actFront);
      menu->addAction(actBack);
      menu->addSeparator();
      menu->addAction(actPerspective);
      menu->addSeparator();
      menu->addAction(actReset);

      connect(actTop, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actBottom, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actRight, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actLeft, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actFront, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actBack, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              trackball, SLOT(setPosition(int)));
      connect(actPerspective, &QAction::triggered,
              this, &MainWindow::togglePerspective);
      connect(actReset, &QAction::triggered,
              this, &MainWindow::resetCamera);
    }

    // Light dialog
    {

    }

    // Help dialog
    {
      QAction * actHelp = new QAction("&Help", mnbMenu);
      actHelp->setShortcutContext(Qt::ApplicationShortcut);
      actHelp->setShortcut(QKeySequence(Qt::Key_Question));
      connect(actHelp, &QAction::triggered,
              this, &MainWindow::showHelp);
      mnbMenu->addAction(actHelp);
    }

    setMenuBar(mnbMenu);
  }

  void MainWindow::togglePerspective() {
    if (ortho) {
      actPerspective->setText("Orthographic");
      actPerspective->setIcon(QIcon(":images/ortho.png"));
      ortho = false;
    } else {
      actPerspective->setText("Perspective");
      actPerspective->setIcon(QIcon(":images/perspective.png"));
      ortho = true;
    }

    trackball->togglePerspective();
  }

  void MainWindow::resetCamera() {
    actPerspective->setText("Orthographic");
    actPerspective->setIcon(QIcon(":images/ortho.png"));
    ortho = false;

    trackball->reset();
  }

  void MainWindow::showHelp() {
    HelpDialog help(this);
    help.exec();
  }

  void MainWindow::toggleFullscreen() {
    full = !full;
    if (full) {
      showFullScreen();
    } else {
      showNormal();
    }
  }
}
