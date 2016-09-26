#include "Trackball.hh"
#include "Sdl.hh"

#include <glm/gtc/matrix_transform.hpp>

Vec3 Trackball::surfaceVector() {
    auto screen = Sdl::screenCoords();
    float width = screen.x / 2.0f;
	float height = screen.y / 2.0f;

	Vec3 point(Sdl::mouseCoords(), 0.0f);
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

Trackball::Trackball():
    mFov(45.0f),
    mScale{ 1.0f, 1.0f, 1.0f },
    projectionDirty(true),
    viewDirty(true)
{
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
	mCurrentRotation = glm::rotate(mCurrentRotation, 0.025f * x, glm::conjugate(mCurrentRotation) * Vec3(0, -1, 0));
	mCurrentRotation = glm::rotate(mCurrentRotation, 0.025f * y, glm::conjugate(mCurrentRotation) * Vec3(-1, 0, 0));

    viewDirty = true;
}

void Trackball::translate(int x, int y)
{
	mTranslation.x += 0.01f * x;
	mTranslation.y -= 0.01f * y;
    viewDirty = true;
}

void Trackball::zoom(int x, int y)
{
	mScale -= 0.05f * y;
    viewDirty = true;
}

/*
For some reason, this function messes with the conjugate calculation
Have to dig a bit into the implementation to see what's going on

Disabled for now :/
*/
void Trackball::fov(int x, int y)
{
	//mFov += y;
	//mProjection = glm::perspectiveFov(glm::radians(mFov), (float) mWidth, (float) mHeight, 0.1f, 100.0f);
	//mDirtyValues |= projection_dirty;
}

Mat4 Trackball::rotationMatrix() {
	return glm::scale(glm::translate(Mat4(mCurrentRotation), glm::conjugate(mCurrentRotation) * mTranslation), mScale);
}
