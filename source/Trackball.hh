#ifndef __INF251_TRACKBALL__54845665
#define __INF251_TRACKBALL__54845665

#include "infdef.hh"

class Trackball {
private:

  // View tracking variables
  Vec3 mInitialPosition;
  Vec3 mCurrentPosition;
  Quat mInitialRotation;

  // Light tracking variables
  Vec3 mInitialLightPos;

  // Camera reference
  Quat * mRotation;
  Vec3 * mLightPosition;
  bool * mViewDirty;

public:
  float mSensitivityTranslation;

  Trackball(Quat * rotation, Vec3 * lightPosition, bool * viewDirty);

  // Convert the mouse click to a spherical vector
  Vec3 surfaceVector(int x, int y);

  void anchor(int x, int y);
  void rotate(int x, int y);
  void rotateLight(int x, int y);
  void resize(int width, int height);

};

#endif //__INF251_TRACKBALL__54845665
