#include "Trackball.hh"

#include <glm/gtc/matrix_transform.hpp>

namespace {
  int _width = 1;
  int _height = 1;
}

Vec3 Trackball::surfaceVector(int x, int y) {
  float width = _width / 2.0f;
  float height = _height / 2.0f;

  Vec3 point(x, y, 0.0f);
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
  mCurrentLightPos{ 1.0f, 1.0f, 1.0f },
  mTranslation{ 0.0f, 0.0f, -5.0f },
  mSensitivityTranslation(0.005f),
  mSensitivityZooming(0.025f)
{
}

void Trackball::anchor(int x, int y) {
#ifdef SPHERICAL_TRACKBALL
  mInitialPosition = surfaceVector(x, y);
  mAnchor.x = x;
  mAnchor.y = y;
  mInitialRotation = mCurrentRotation;
  mInitialLightPos = mCurrentLightPos;
#endif
}

void Trackball::rotate(int x, int y) {
#ifdef SPHERICAL_TRACKBALL
  mCurrentPosition = surfaceVector(x, y);

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

void Trackball::rotateLight(int x, int y) {
#ifdef SPHERICAL_TRACKBALL
  mCurrentPosition = surfaceVector(x, y);

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

void Trackball::translate(int x, int y) {
  mTranslation.x += mSensitivityTranslation * (x - mAnchor.x);
  mTranslation.y -= mSensitivityTranslation * (y - mAnchor.y);
  anchor(x, y);
  viewDirty = true;
}

void Trackball::zoom(int amount) {
  mFov += mSensitivityZooming * amount;
  if (mFov < 1.0f) {
    mFov = 1.0f;
  } else if (mFov >= 180.0f) {
    mFov = 179.0f;
  }
  projectionDirty = true;
}

void Trackball::reset() {
  mOrtho = false;
  mFov = 45.0f;
  projectionDirty = true;

  mCurrentRotation = Quat();
  mTranslation = Vec3(0.0f, 0.0f, -5.0f);
  viewDirty = true;

  mCurrentLightPos = Vec3(1.0f, 1.0f, 1.0f);
  lightDirty = true;
}

void Trackball::togglePerspective() {
  mOrtho = !mOrtho;
  projectionDirty = true;
}

void Trackball::setPosition(int position) {
  mTranslation = Vec3(0.0f, 0.0f, -5.0f);
  mCurrentRotation = Quat();
  viewDirty = true;

  switch (position) {
    case 0: // Top
      mCurrentRotation = glm::rotate(
        mCurrentRotation,
        glm::pi<float>() / 2.0f,
        Vec3(1.0f, 0.0f, 0.0f));
      break;
    case 1: // Bottom
      mCurrentRotation = glm::rotate(
        mCurrentRotation,
        -glm::pi<float>() / 2.0f,
        Vec3(1.0f, 0.0f, 0.0f));
      break;
    case 2: // Right
      mCurrentRotation = glm::rotate(
        mCurrentRotation,
        glm::pi<float>() / 2.0f,
        Vec3(0.0f, 1.0f, 0.0f));
      break;
    case 3: // Left
      mCurrentRotation = glm::rotate(
        mCurrentRotation,
        -glm::pi<float>() / 2.0f,
        Vec3(0.0f, 1.0f, 0.0f));
      break;
    case 4: // Front
    default:
      break;
    case 5: // Back
      mCurrentRotation = glm::rotate(
        mCurrentRotation,
        glm::pi<float>(),
        Vec3(0.0f, 1.0f, 0.0f));
      break;
  }
}

Mat4 Trackball::rotation() {
  return glm::translate(Mat4(mCurrentRotation), glm::conjugate(mCurrentRotation) * mTranslation);
}

Mat4 Trackball::projection() {
  if (mOrtho) {
    float zoom = mFov / 22.5f;
    float ratio = static_cast<float>(_height) / static_cast<float>(_width);
    return glm::ortho(-zoom, zoom, -zoom * ratio, zoom * ratio, 1.0f, 20.0f);
  } else {
    return glm::perspectiveFov(glm::radians(mFov), static_cast<float>(_width), static_cast<float>(_height), 1.0f, 20.0f);
  }
}

Vec3 Trackball::lightPosition() {
  return glm::normalize(mCurrentLightPos);
}

void Trackball::resize(int width, int height) {
  _width = width;
  _height = height;
  projectionDirty = true;
}

Vec3 Trackball::eyePosition() {
  return mCurrentRotation * mTranslation;
}
