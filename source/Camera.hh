#ifndef __INF251_CAMERA__53672421
#define __INF251_CAMERA__53672421

#include "infdef.hh"
#include "Trackball.hh"
#include "CameraPath.hh"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

class Camera : public QObject {
  Q_OBJECT

public:
  enum Mode {
    TRACKBALL,
    WASD,
    PATH
  };

  enum Position {
    TOP,
    BOTTOM,
    RIGHT,
    LEFT,
    FRONT,
    BACK
  };

  explicit Camera();
  ~Camera() = default;

  bool projectionDirty;
  bool viewDirty;
  bool lightDirty;

  // Transform matrices
  Mat4 rotation();
  Mat4 projection();

  // Managed locations
  Vec3 lightPosition();
  Vec3 eyePosition();

  void moveTo(const Vec3 & position);
  void lookAt(const Vec3 & target);
  void update();
  void stop();
  void setMode(Mode mode);
  void setDefaultPosition(Position position);

  public slots:
  void mousePressed(QMouseEvent *evt);
  void mouseMoved(QMouseEvent *evt);
  void wheelMoved(QWheelEvent *evt);
  void keyPressed(QKeyEvent *evt);
  void keyReleased(QKeyEvent *evt);
  void setMode(int mode) {
    setMode(static_cast<Mode>(mode & 3));
  }

  void reset();
  void zoom(int amount);
  void togglePerspective();
  void setDefaultPosition(int position) {
    setDefaultPosition(static_cast<Position>(position % 6));
  }
  void resize(int width, int height);
  void translate(int x, int y);

private:

  // Mode tracking
  Mode mMode = TRACKBALL;

  // View tracking variables
  Vec3 mTranslation;
  Quat mRotation;

  // Light tracking variables
  Vec3 mLightPosition;

  // Projection tracking variables
  float mFOV;
  bool mOrtho;

  // Control variables
  float mZoomSensitivity;
  float mTranslationSensitivity;

  Trackball trackball;
  CameraPath path;
};

#endif //__INF251_CAMERA__53672421
