#ifndef __INF251_TRACKBALL__54845665
#define __INF251_TRACKBALL__54845665

#include "infdef.hh"

class Trackball {
	// Trackball status flags
	uint8_t mDirtyValues;

	// Mouse tracking variables
	int mXPos;
	int mYPos;

	// Viewport variables
	unsigned int mWidth;
	unsigned int mHeight;

	// View tracking variables
	Vec3 mInitialPosition;
	Vec3 mCurrentPosition;
	Quat mInitialRotation;
	Quat mCurrentRotation;
	Vec3 mScale;
	Vec3 mTranslation;

	// Projection tracking variables
	float mFov;
	Mat4 mProjection;

	// Convert the mouse click to a spherical vector
	Vec3 getSurfaceVector();

public:
	// Bitmask for tracking trackball changes
	static constexpr uint8_t model_dirty = 1 << 0;
	static constexpr uint8_t view_dirty = 1 << 1;
	static constexpr uint8_t projection_dirty = 1 << 2;
	static constexpr uint8_t zoom_dirty = 1 << 3;
	static constexpr uint8_t ortho_dirty = 1 << 4;
	static constexpr uint8_t fov_dirty = 1 << 5;

	Trackball();
	
	void setSize(unsigned int width, unsigned int height);

	void mousePressed(int x, int y);
	void mouseReleased();
	void rotate(int x, int y);
	void translate(int x, int y);
	void zoom(int x, int y);
	void fov(int x, int y);

	Mat4 getRotation();

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

	void clearDirtyFlag(uint8_t flags) {
		if (flags <= getAllDirtyFlags()) {
			mDirtyValues &= ~flags;
		}
	}

	uint8_t getDirtyValues() {
		return mDirtyValues;
	}

	Mat4 getProjection() {
		return mProjection;
	}

};

#endif //__INF251_TRACKBALL__54845665
