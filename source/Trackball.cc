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

	point.z = glm::sqrt(glm::length(point));

	return glm::normalize(point);
}

Trackball::Trackball() :
	mFov(45.0f),
	projectionDirty(true),
	viewDirty(true),
	mTranslation{ 0.0f, 0.0f, -5.0f },
	mSensitivityRotation(0.0025f),
	mSensitivityTranslation(0.01f),
	mSensitivityZooming(1.0f)
{
}

void Trackball::mousePressed(int x, int y)
{
#ifdef SPHERICAL_TRACKBALL
	mInitialPosition = surfaceVector();
	mInitialRotation = mCurrentRotation;
#endif
}

void Trackball::mouseReleased() {

}

void Trackball::rotate(int x, int y)
{
#ifdef SPHERICAL_TRACKBALL
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
	mFov += mSensitivityZooming * y;
	if (mFov < 1.0f) {
		mFov = 1.0f;
	}
	else if (mFov >= 180.0f) {
		mFov = 179.0f;
	}
	projectionDirty = true;
}

void Trackball::reset()
{
	auto screen = Sdl::screenCoords();
	mOrtho = false;
	projectionDirty = true;

	mCurrentRotation = Quat();
	mTranslation = Vec3(0.0f, 0.0f, -5.0f);
	mFov = 45.0f;
	viewDirty = true;
}

void Trackball::togglePerspective()
{
	mOrtho = !mOrtho;
	projectionDirty = true;
}

Mat4 Trackball::rotationMatrix() {
	return glm::translate(Mat4(mCurrentRotation), glm::conjugate(mCurrentRotation) * mTranslation);
}

Mat4 Trackball::projectionMatrix()
{
	auto screen = Sdl::screenCoords();

	if (mOrtho) {
		return glm::ortho(0.0f, static_cast<float>(screen.x), 0.0f, static_cast<float>(screen.y), 0.1f, 100.0f);
	}
	else {
		return glm::perspectiveFov(glm::radians(mFov), static_cast<float>(screen.x), static_cast<float>(screen.y), 0.1f, 100.0f);
	}
}
