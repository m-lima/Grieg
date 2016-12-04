#include "MainWindow.hh"
#include "HelpDialog.hh"

#include <memory>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QSignalMapper>
#include <QShortcut>

namespace View {
  MainWindow::MainWindow() {
    QStatusBar *status = new QStatusBar(this);

    lblFPS = new QLabel(status);
    lblPosition = new QLabel(status);

    QWidget * pnlAmbient = new QWidget(status);
    QHBoxLayout * lytAmbient = new QHBoxLayout(pnlAmbient);
    lytAmbient->setMargin(0);
    lytAmbient->setSpacing(0);
    QLabel *lblAmbient = new QLabel("Ambient: ", status);
    lblAmbient->setAlignment(Qt::AlignVCenter);
    sldAmbient = new QSlider(status);

    lblFPS->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    lblPosition->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    sldAmbient->setValue(40);
    sldAmbient->setMaximum(100);
    sldAmbient->setOrientation(Qt::Orientation::Horizontal);

    lytAmbient->addWidget(lblAmbient);
    lytAmbient->addWidget(sldAmbient);

    status->addPermanentWidget(lblFPS, 1);
    status->addPermanentWidget(lblPosition, 1);
    status->addPermanentWidget(pnlAmbient, 2);

    setStatusBar(status);
  }

  void MainWindow::attachRenderer(Renderer * renderer) {
    if (mRenderer == nullptr) {
      mRenderer = renderer;
      mRenderer->setFPS(lblFPS);
      mRenderer->setPosition(lblPosition);

      camera = &(mRenderer->camera);

      buildMenu();

      setCentralWidget(renderer);
      resetCamera();
    }
  }

  void MainWindow::buildMenu() {
    QMenuBar *mnbMenu = new QMenuBar(this);
    QMenu *menu;
    QActionGroup *group;
    QSignalMapper *mapper;

    // File menu
    {
      menu = mnbMenu->addMenu("&File");

      QAction *actFull = new QAction("&Fullscreen", menu);
      QAction *actExit = new QAction("&Exit", menu);

      actFull->setIcon(QIcon(":images/full.png"));
      actExit->setIcon(QIcon(":images/exit.png"));

      actFull->setShortcutContext(Qt::ApplicationShortcut);
      actExit->setShortcutContext(Qt::ApplicationShortcut);

      actFull->setShortcut(QKeySequence(Qt::Key_F));
      actExit->setShortcut(QKeySequence(Qt::Key_Escape));

      connect(actFull, &QAction::triggered, this, &MainWindow::toggleFullscreen);
      connect(actExit, &QAction::triggered, this, &QMainWindow::close);

      menu->addAction(actFull);
      menu->addSeparator();
      menu->addAction(actExit);
    }

    // Model menu
    {
      menu = mnbMenu->addMenu("&Model");

      QAction *actGrieg = new QAction("&Grieghallen", menu);
      QAction *actSuzy = new QAction("Big &Suzy", menu);
      QAction *actRotate = new QAction("&Rotate", menu);

      actGrieg->setCheckable(true);
      actSuzy->setCheckable(true);
      actRotate->setEnabled(false);

      group = new QActionGroup(menu);
      group->addAction(actGrieg);
      group->addAction(actSuzy);
      actGrieg->setChecked(true);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actGrieg, 0);
      mapper->setMapping(actSuzy, 1);

      actRotate->setCheckable(true);
      actRotate->setChecked(false);
      actRotate->setShortcutContext(Qt::ApplicationShortcut);
      actRotate->setShortcut(QKeySequence(Qt::Key_R));

      connect(actGrieg, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actSuzy, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              mRenderer, SLOT(setModel(int)));
      connect(actRotate, SIGNAL(triggered(bool)),
              mRenderer, SLOT(setModelRotation(bool)));
      connect(actSuzy, &QAction::toggled,
              actRotate, &QAction::setEnabled);

      menu->addAction(actGrieg);
      menu->addAction(actSuzy);
      menu->addSeparator();
      menu->addAction(actRotate);
    }

    // Shader menu
    {
      menu = mnbMenu->addMenu("&Shader");

      QAction *actBasic = new QAction("Basic", menu);
      QAction *actAmbient = new QAction("Ambient", menu);
      QAction *actNormals = new QAction("Normals", menu);
      QAction *actHeight = new QAction("Height", menu);
      QAction *actToon = new QAction("Toon", menu);
      QAction *actTilt = new QAction("Tilt-shift", menu);

      actBasic->setCheckable(true);
      actAmbient->setCheckable(true);
      actNormals->setCheckable(true);
      actHeight->setCheckable(true);
      actToon->setCheckable(true);
      actTilt->setCheckable(true);

      group = new QActionGroup(menu);
      group->addAction(actBasic);
      group->addAction(actAmbient);
      group->addAction(actNormals);
      group->addAction(actHeight);
      group->addAction(actToon);
      group->addAction(actTilt);
      actBasic->setChecked(true);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actBasic, 0);
      mapper->setMapping(actAmbient, 1);
      mapper->setMapping(actNormals, 2);
      mapper->setMapping(actHeight, 3);
      mapper->setMapping(actToon, 4);
      mapper->setMapping(actTilt, 5);

      menu->addAction(actBasic);
      menu->addAction(actAmbient);
      menu->addAction(actNormals);
      menu->addAction(actHeight);
      menu->addAction(actToon);
      menu->addAction(actTilt);

      connect(actBasic, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actAmbient, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actNormals, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actHeight, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actToon, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actTilt, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              mRenderer, SLOT(setShader(int)));

      menu->addAction(actBasic);
      menu->addAction(actToon);
      menu->addAction(actTilt);
    }

    // Camera menu
    {
      menu = mnbMenu->addMenu("&Camera");

      actTrack = new QAction("&Trackball", menu);
      QAction *actWASD = new QAction("&Wasd", menu);
      QAction *actPath = new QAction("&Path", menu);
      QAction *actTop = new QAction("&Top", menu);
      QAction *actBottom = new QAction("&Bottom", menu);
      QAction *actRight = new QAction("Rig&ht", menu);
      QAction *actLeft = new QAction("L&eft", menu);
      QAction *actFront = new QAction("Fro&nt", menu);
      QAction *actBack = new QAction("Bac&k", menu);
      actPerspective = new QAction("Orthographic", menu);
      QAction *actReset = new QAction("Re&set", menu);

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

      actTrack->setCheckable(true);
      actWASD->setCheckable(true);
      actPath->setCheckable(true);

      group = new QActionGroup(menu);
      group->addAction(actTrack);
      group->addAction(actWASD);
      group->addAction(actPath);
      actTrack->setChecked(true);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actTop, 0);
      mapper->setMapping(actBottom, 1);
      mapper->setMapping(actRight, 2);
      mapper->setMapping(actLeft, 3);
      mapper->setMapping(actFront, 4);
      mapper->setMapping(actBack, 5);

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
              camera, SLOT(setDefaultPosition(int)));
      connect(actPerspective, &QAction::triggered,
              this, &MainWindow::togglePerspective);
      connect(actReset, &QAction::triggered,
              this, &MainWindow::resetCamera);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actTrack, 0);
      mapper->setMapping(actWASD, 1);
      mapper->setMapping(actPath, 2);

      connect(actTrack, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actWASD, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actPath, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              camera, SLOT(setMode(int)));
      connect(actTrack, &QAction::toggled,
              actPerspective, &QAction::setEnabled);

      QMenu * subMenu = new QMenu("&Mode", menu);
      subMenu->addAction(actTrack);
      subMenu->addAction(actWASD);
      subMenu->addAction(actPath);
      menu->addMenu(subMenu);
      menu->addSeparator();
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
    }

    // Light menu
    {
      menu = mnbMenu->addMenu("&Lights");

      QAction *actSun = new QAction("&Sun", menu);
      QAction *actSpot1 = new QAction("Light &1", menu);
      QAction *actSpot2 = new QAction("Light &2", menu);
      QAction *actRotate = new QAction("Rotate", menu);

      actRotate->setShortcutContext(Qt::ApplicationShortcut);
      actRotate->setShortcut(QKeySequence(Qt::Key_L));
      actRotate->setCheckable(true);
      actRotate->setChecked(true);

      mapper = new QSignalMapper(this);
      mapper->setMapping(actSun, 0);
      mapper->setMapping(actSpot1, 1);
      mapper->setMapping(actSpot2, 2);

      connect(actSun, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actSpot1, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(actSpot2, SIGNAL(triggered()),
              mapper, SLOT(map()));
      connect(mapper, SIGNAL(mapped(int)),
              mRenderer, SLOT(showPanel(int)));
      connect(actRotate, SIGNAL(triggered(bool)),
              mRenderer, SLOT(rotateLights(bool)));

      menu->addAction(actSun);
      menu->addAction(actSpot1);
      menu->addAction(actSpot2);
      menu->addSeparator();
      menu->addAction(actRotate);
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

    connect(sldAmbient, &QSlider::valueChanged, mRenderer, &Renderer::setAmbient);
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

    camera->togglePerspective();
  }

  void MainWindow::resetCamera() {
    actPerspective->setText("Orthographic");
    actPerspective->setIcon(QIcon(":images/ortho.png"));
    ortho = false;
    actTrack->setChecked(true);

    camera->reset();
  }

  void MainWindow::showHelp() {
    HelpDialog help(this);
    help.exec();
  }

  void MainWindow::toggleFullscreen() {
    if (isFullScreen()) {
      showNormal();
    } else {
      showFullScreen();
    }
  }

  void MainWindow::keyPressEvent(QKeyEvent *evt) {
    camera->keyPressed(evt);
  }

  void MainWindow::keyReleaseEvent(QKeyEvent *evt) {
    if (evt->key() == Qt::Key_Escape) {
      static_cast<QWidget*>(parent())->close();
    } else {
      camera->keyReleased(evt);
    }
  }

  bool MainWindow::event(QEvent * evt) {
    if (evt->type() == QEvent::WindowDeactivate) {
      camera->stop();
    }

    return QMainWindow::event(evt);
  }

}
