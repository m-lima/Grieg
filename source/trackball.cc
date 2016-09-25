#include "trackball.hh"
#include "SDL.h"

Vec3 Trackball::getSurfaceVector(unsigned int x, unsigned int y) {
	float width = mWidth / 2.0f;
	float height = mHeight / 2.0f;

	Vec3 point(x, y, 0);

	point.x -= width;
	point.y -= height;

	point.x /= width;
	point.y /= -height;

	float length = glm::length(point);
	if (length >= 1.0f) {
		point.y = 0.0f;
	}
	else {
		point.y = glm::sqrt(1.0f - length);
	}

	return glm::normalize(point);
}

Trackball::Trackball()
{
	mDirtyValues = getAllDirtyFlags();
}

void Trackball::mousePressed(const uint8_t button, const unsigned int mods, const unsigned int x, const unsigned int y)
{
	// Right now, we only accept CTRL
	// Quit and do nothing if other MODS are pressed (or else will break once
	// they are released)
	if (mods & ~(KMOD_SHIFT | KMOD_ALT)) {
		return;
	}

	mXPos = x;
	mYPos = y;

	if (button & SDL_BUTTON_RIGHT) {

	} else if (button & SDL_BUTTON_MIDDLE) {

	} else if (button & SDL_BUTTON_LEFT) {
		mInitialPosition = getSurfaceVector(x, y);
		mInitialRotation = mCurrentRotation;
	}
}

void Trackball::mouseDragged(const uint8_t button, const unsigned int mods, const unsigned int x, const unsigned int y)
{
	if (mods & ~(KMOD_SHIFT | KMOD_ALT)) {
		return;
	}

	mXPos = x;
	mYPos = y;

	if (button & SDL_BUTTON_RIGHT) {

	}
	else if (button & SDL_BUTTON_MIDDLE) {

	}
	else if (button & SDL_BUTTON_LEFT) {
		mCurrentPosition = getSurfaceVector(x, y);
		float angle = glm::dot(mInitialPosition, mCurrentPosition);
		mAxis = glm::cross(mInitialPosition, mCurrentPosition);
		mAxis = glm::normalize(mAxis);

		mCurrentRotation = glm::rotate(glm::conjugate(mInitialRotation), angle, mAxis);

		mDirtyValues |= model_dirty;
	}
}
