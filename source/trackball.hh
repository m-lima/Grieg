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

	// Viewport variables
	unsigned int mWidth;
	unsigned int mHeight;

	// View variables
	Vec3 mEyePosition;
	Vec3 mTargetPostion;
	Vec3 mUpVector;

	// Trackball tracking variables
	Vec3 mInitialPosition;
	Vec3 mCurrentPosition;
	Vec3 mAxis;

	Quat mInitialRotation;
	Quat mCurrentRotation;

	// Convert the mouse click to a spherical vector
	Vec3 getSurfaceVector(unsigned int x, unsigned int y);

public:
	Trackball();
	~Trackball();

	void setSize(unsigned int width, unsigned int height) {
		mWidth = width;
		mHeight = height;
	}

	void mousePressed(const uint8_t button, const unsigned int mods,
		const unsigned int x, const unsigned int y);

	void mouseReleased(const uint8_t button, const unsigned int mods, const unsigned int x, const unsigned int y);

	void mouseDragged(const uint8_t button, const unsigned int mods,
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

	void clearDirtyFlag(uint8_t flags) {
		if (flags <= getAllDirtyFlags()) {
			mDirtyValues &= ~flags;
		}
	}

	uint8_t getDirtyValues() {
		return mDirtyValues;
	}

	glm::fmat4 getRotation() {
		return glm::fmat4(mCurrentRotation);
	}

};

#endif //__INF251_TRACKBALL__54845665
