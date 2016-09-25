#ifndef __INF251_TRACKBALL__54845665
#define __INF251_TRACKBALL__54845665

#include "infdef.hh"

class Trackball {
  // Bitmask for tracking trackball changes
  const uint8_t model_dirty =       1 << 0;
  const uint8_t view_dirty =        1 << 1;
  const uint8_t projection_dirty =  1 << 2;
  const uint8_t zoom_dirty =        1 << 3;
  const uint8_t ortho_dirty =       1 << 4;
  const uint8_t fov_dirty =         1 << 5;

  // Trackball status flags
  uint8_t mDirtyValues;

  // View variables
  Vec3 mEyePosition;
  Vec3 mTargetPostion;
  Vec3 mUpVector;

  // Trackball tracking variables
  Vec3 mInitialMousePosition;
  Vec3 mCurrentMousePosition;
  Vec3 mMouseMoveAxis;

  Quat mInitialRotation;
  Quat mCurrentRotation;

public:
  Trackball();

  inline uint8_t getAllDirtyFlags() {
      return model_dirty |
              view_dirty |
              projection_dirty |
              zoom_dirty |
              ortho_dirty |
              fov_dirty;
  }

  inline void markAllDirty() {
      mDirtyValues = getAllDirtyFlags();
  }

};

#endif //__INF251_TRACKBALL__54845665
