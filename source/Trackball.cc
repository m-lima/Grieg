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

	point.x /= -width;
	point.y /= height;

	point.z = glm::sqrt(glm::length(point));

	return glm::normalize(point);
}

Trackball::Trackball() :
	mFov(45.0f),
	mScale{ 1.0f, 1.0f, 1.0f },
	projectionDirty(true),
	viewDirty(true),
	mSensitivityRotation(0.025f),
	mSensitivityTranslation(0.01f),
	mSensitivityZooming(0.05f),
	mSensitivityFov(0.5f)
{
}

void Trackball::mousePressed(int x, int y)
{
	mInitialPosition = surfaceVector();
	mInitialRotation = mCurrentRotation;
}

void Trackball::mouseReleased() {

}

/**
  This rotation is not very good. It is impossible to rotate around the Z axis
  But a trackball would not allow for linear rotation
 **/
void Trackball::rotate(int x, int y)
{
#if 0
	mCurrentPosition = surfaceVector();

	float angle = glm::acos(glm::dot(mInitialPosition, mCurrentPosition));
	Vec3 mAxis = glm::cross(mInitialPosition, mCurrentPosition);
	mAxis = glm::normalize(mAxis);

	mCurrentRotation = glm::rotate(mInitialRotation, angle, glm::conjugate(mInitialRotation) * mAxis);
#else
	mCurrentRotation = glm::rotate(mCurrentRotation, mSensitivityRotation * x, glm::conjugate(mCurrentRotation) * Vec3(0, -1, 0));
	mCurrentRotation = glm::rotate(mCurrentRotation, mSensitivityRotation * y, glm::conjugate(mCurrentRotation) * Vec3(-1, 0, 0));
#endif

	viewDirty = true;
}

void Trackball::translate(int x, int y)
{
	mTranslation.x += mSensitivityTranslation * x;
	mTranslation.y -= mSensitivityTranslation * y;
	viewDirty = true;
}

void Trackball::zoom(int x, int y)
{
	mScale -= mSensitivityZooming * y;
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

void Trackball::reset()
{
	mCurrentRotation = Quat();
	mTranslation = Vec3();
	mScale = Vec3(1.0f, 1.0f, 1.0f);
	viewDirty = true;
}

Mat4 Trackball::rotationMatrix() {
	return glm::scale(glm::translate(Mat4(mCurrentRotation), glm::conjugate(mCurrentRotation) * mTranslation), mScale);
}
