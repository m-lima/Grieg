#ifndef __INF251_TRACKBALL__54845665
#define __INF251_TRACKBALL__54845665

#include "infdef.hh"

class Trackball : public QObject {
  Q_OBJECT

private:

  // View tracking variables
  Vec3 mInitialPosition;
  Vec3 mCurrentPosition;
  Quat mInitialRotation;
  Quat mCurrentRotation;
  Vec3 mTranslation;

  // Projection tracking variables
  float mFov;
  bool mOrtho;

  // Light tracking variables
  Vec3 mInitialLightPos;
  Vec3 mCurrentLightPos;

  Vec2 mAnchor;

public:
  // Also a bitmask, but using C++11
  bool projectionDirty : 1;
  bool viewDirty : 1;
  bool lightDirty : 1;

  float mSensitivityTranslation;
  float mSensitivityZooming;

  Trackball();

  // Convert the mouse click to a spherical vector
  Vec3 surfaceVector(int x, int y);

  Mat4 rotation();
  Mat4 projection();
  Vec3 lightPosition();

  void resize(int width, int height);

  public slots:

  void anchor(int x, int y);
  void rotate(int x, int y);
  void rotateLight(int x, int y);
  void translate(int x, int y);
  void zoom(int amount);
  void reset();
  void togglePerspective();
  void setPosition(int position);

};

#endif //__INF251_TRACKBALL__54845665
