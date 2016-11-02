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
	lightDirty(true),
	mCurrentLightPos{1.0f, 1.0f, 1.0f},
	mTranslation{ 0.0f, 0.0f, -5.0f },
	mSensitivityTranslation(0.005f),
	mSensitivityZooming(1.0f)
{
}

void Trackball::anchorRotation(int x, int y)
{
#ifdef SPHERICAL_TRACKBALL
	mInitialPosition = surfaceVector();
	mInitialRotation = mCurrentRotation;
	mInitialLightPos = mCurrentLightPos;
#endif
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
	mCurrentRotation = glm::rotate(mCurrentRotation, glm::radians(static_cast<float>(x)), glm::conjugate(mCurrentRotation) * Vec3(0, 1, 0));
	mCurrentRotation = glm::rotate(mCurrentRotation, glm::radians(static_cast<float>(y)), glm::conjugate(mCurrentRotation) * Vec3(1, 0, 0));
#endif

	viewDirty = true;
}

void Trackball::rotateLight(int x, int y)
{
#ifdef SPHERICAL_TRACKBALL
	mCurrentPosition = surfaceVector();

	float angle = glm::acos(glm::dot(mInitialPosition, mCurrentPosition));
	Vec3 mAxis = glm::cross(mInitialPosition, mCurrentPosition);
	mAxis = glm::normalize(mAxis);

	Quat lightRotation = glm::rotate(Quat(), angle, mAxis);
	mCurrentLightPos = lightRotation * mInitialLightPos;
#else
	Quat lightRotation = glm::rotate(Quat(), glm::radians(static_cast<float>(x)), Vec3(0, 1, 0));
	lightRotation = glm::rotate(lightRotation, glm::radians(static_cast<float>(y)), glm::conjugate(lightRotation) * Vec3(1, 0, 0));
	mCurrentLightPos = lightRotation * mCurrentLightPos;
#endif

	lightDirty = true;
}

void Trackball::translate(int x, int y)
{
	mTranslation.x += mSensitivityTranslation * x;
	mTranslation.y -= mSensitivityTranslation * y;
	viewDirty = true;
}

void Trackball::zoom(int amount)
{
	mFov += mSensitivityZooming * amount;
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
	mOrtho = false;
	mFov = 45.0f;
	projectionDirty = true;

	mCurrentRotation = Quat();
	mTranslation = Vec3(0.0f, 0.0f, -5.0f);
	viewDirty = true;

	mCurrentLightPos = Vec3(1.0f, 1.0f, 1.0f);
	lightDirty = true;
}

void Trackball::togglePerspective()
{
	mOrtho = !mOrtho;
	projectionDirty = true;
}

Mat4 Trackball::rotation() {
	return glm::translate(Mat4(mCurrentRotation), glm::conjugate(mCurrentRotation) * mTranslation);
}

Mat4 Trackball::projection()
{
	auto screen = Sdl::screenCoords();

	if (mOrtho) {
		float zoom = mFov / 45.0f;
		float ratio = static_cast<float>(screen.x >> 1) / screen.y;
		zoom *= 2.0f;
		return glm::ortho(-zoom, zoom, -zoom * ratio, zoom * ratio, 0.0f, 100.0f);
	}
	else {
		return glm::perspectiveFov(glm::radians(mFov), static_cast<float>(screen.x), static_cast<float>(screen.y), 0.1f, 100.0f);
	}
}

Vec3 Trackball::lightPosition()
{
	//Vec3 yo = glm::normalize(mCurrentLightPos);
	//println("X: {} | Y: {} | Z: {}", yo.x, yo.y, yo.z);
	return glm::normalize(mCurrentLightPos);
}
