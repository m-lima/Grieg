#include "trackball.hh"
#include "SDL.h"

void Trackball::getSurfaceVector(unsigned int x, unsigned int y, unsigned int width, unsigned int height, float point[]) {
	width /= 2;
	height /= 2;

	point[0] = (float) x;
	point[1] = (float) y;

	point[0] -= width;
	point[1] -= height;

	point[0] /= width;
	point[1] /= -height;

	float length = point[0] * point[0] + point[1] * point[1];
	if (length >= 1.0f) {
		point[2] = 0.0f;
	}
	else {
		point[2] = glm::sqrt(1.0f - length);
	}

	//glm::normalize(point);
}

Trackball::Trackball()
{
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

	}
}
