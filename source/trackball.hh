#ifndef __INF251_TRACKBALL__54845665
#define __INF251_TRACKBALL__54845665

#include "infdef.hh"

class Trackball {
	// Bitmask for tracking trackball changes
	static constexpr uint8_t model_dirty = 1 << 0;
	static constexpr uint8_t view_dirty = 1 << 1;
	static constexpr uint8_t projection_dirty = 1 << 2;
	static constexpr uint8_t zoom_dirty = 1 << 3;
	static constexpr uint8_t ortho_dirty = 1 << 4;
	static constexpr uint8_t fov_dirty = 1 << 5;

	// Trackball status flags
	uint8_t mDirtyValues;

	// Mouse tracking variables
	unsigned int mXPos;
	unsigned int mYPos;
	float mXPosOld;
	float mYPosOld;

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

	// Convert the mouse click to a spherical vector
	void getSurfaceVector(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float point[]);

public:
	Trackball();
	~Trackball();

	void mousePressed(const uint8_t button, const unsigned int mods,
		const unsigned int x, const unsigned int y);

	uint8_t getAllDirtyFlags() {
		return model_dirty |
			view_dirty |
			projection_dirty |
			zoom_dirty |
			ortho_dirty |
			fov_dirty;
	}

	void markAllDirty() {
		mDirtyValues = getAllDirtyFlags();
	}

	uint8_t getDirtyValues() {
		return mDirtyValues;
	}

};

#endif //__INF251_TRACKBALL__54845665
