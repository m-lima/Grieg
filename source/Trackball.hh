#ifndef __INF251_TRACKBALL__54845665
#define __INF251_TRACKBALL__54845665

#include "infdef.hh"

class Trackball
{
	// View tracking variables
	Vec3 mInitialPosition;
	Vec3 mCurrentPosition;
	Quat mInitialRotation;
	Quat mCurrentRotation;
	Vec3 mTranslation;

	// Projection tracking variables
	float mFov;
	bool mOrtho;

public:
	// Also a bitmask, but using C++11
	bool projectionDirty : 1;
	bool viewDirty : 1;

	float mSensitivityRotation;
	float mSensitivityTranslation;
	float mSensitivityZooming;

	Trackball();

	void mousePressed(int x, int y);
	void mouseReleased();
	void rotate(int x, int y);
	void translate(int x, int y);
	void zoom(int x, int y);
	void reset();
	void togglePerspective();

	// Convert the mouse click to a spherical vector
	Vec3 surfaceVector();

	Mat4 rotationMatrix();

	Mat4 projectionMatrix();
};

#endif //__INF251_TRACKBALL__54845665
