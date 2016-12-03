#ifndef __INF251_CAMERA__53672421
#define __INF251_CAMERA__53672421

#include "infdef.hh"
#include "Trackball.hh"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

class Camera : public QObject {
  Q_OBJECT

public:
  enum MODE {
    TRACKBALL,
    WASD,
    PATH
  };

  bool projectionDirty : 1;
  bool viewDirty : 1;
  bool lightDirty : 1;
  bool moving();

  // Transform matrices
  Mat4 rotation();
  Mat4 projection();

  // Managed locations
  Vec3 lightPosition();
  Vec3 eyePosition();

  public slots:
  void mousePressed(QMouseEvent *evt);
  void mouseMoved(QMouseEvent *evt);
  void wheelMoved(QWheelEvent *evt);
  void keyPressed(QKeyEvent *evt);
  void keyReleased(QKeyEvent *evt);

  void reset();
  void zoom(int amount);
  void togglePerspective();
  void setPosition(int position);
  void resize(int width, int height);

private:

  // Mode tracking
  MODE mMode = TRACKBALL;

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


  Trackball trackball;


};

#endif //__INF251_CAMERA__53672421
