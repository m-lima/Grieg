#include "trackball.hh"
#include "SDL.h"

#include <glm/gtc/matrix_transform.hpp>

Vec3 Trackball::getSurfaceVector() {
	float width = mWidth / 2.0f;
	float height = mHeight / 2.0f;

	Vec3 point(mXPos, mYPos, 0.0f);
	point.x -= width;
	point.y -= height;

	point.x /= width;
	point.y /= -height;

	point.x = glm::min(0.99f, glm::max(-0.99f, point.x));
	point.y = glm::min(0.99f, glm::max(-0.99f, point.y));

	float xx = point.x * point.x;
	float yy = point.y * point.y;
	//float z = glm::abs((1.0f - xx) - yy);
	point.z = glm::sqrt(glm::length(point));
	println("X: {} | {}\nY: {} | {}\nZ: {}", point.x, xx, point.y, yy, point.z);

	//float length = glm::length(point);
	//println("L: {}", length);
	//if (length >= 1.0f) {
	//	point.z = 0.0f;
	//}
	//else {
	//	point.z = glm::sqrt(1.0f - length);
	//}

	return glm::normalize(point);
}

Trackball::Trackball()
{
	mProjection = glm::perspective(glm::radians(120.0f), 4.0f / 3.0f, 0.1f, 100.f);
	markAllDirty();
}

void Trackball::mousePressed(int x, int y)
{
	//mXPos = x;
	//mYPos = y;

	//mInitialPosition = getSurfaceVector();
	//mInitialRotation = mCurrentRotation;
}

void Trackball::mouseReleased() {

}

/**
	This rotation is not very good. It is impossible to rotate around the Z axis
	But a trackball would not allow for linear rotation
**/
void Trackball::rotate(int x, int y)
{
	//mXPos += x;
	//mYPos += y;

	//mCurrentPosition = getSurfaceVector();
	//
	//float angle = glm::acos(glm::dot(mInitialPosition, mCurrentPosition));
	//Vec3 mAxis = glm::cross(mInitialPosition, mCurrentPosition);
	//mAxis = glm::normalize(mAxis);

	//mCurrentRotation = glm::rotate(mInitialRotation, angle, mAxis);
	//mCurrentRotation = glm::rotate(glm::conjugate(mInitialRotation), angle, mAxis);
	mCurrentRotation = glm::rotate(mCurrentRotation, 0.0025f * x, glm::conjugate(mCurrentRotation) * Vec3(0, -1, 0));
	mCurrentRotation = glm::rotate(mCurrentRotation, 0.0025f * y, glm::conjugate(mCurrentRotation) * Vec3(-1, 0, 0));

	mDirtyValues |= view_dirty;
}

void Trackball::translate(int x, int y)
{
	mTranslation.x += 0.001f * x;
	mTranslation.y -= 0.001f * y;
	mDirtyValues |= view_dirty;
}

Mat4 Trackball::getRotation() {
	return glm::translate(Mat4(mCurrentRotation), glm::conjugate(mCurrentRotation) * mTranslation);
}
