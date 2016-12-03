#include "Trackball.hh"

#include <glm/gtc/matrix_transform.hpp>

namespace {
  int _width = 1;
  int _height = 1;
}

Trackball::Trackball(Quat * rotation, Vec3 * lightPosition) :
  mRotation(rotation),
  mLightPosition(lightPosition) {}

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

void Trackball::anchor(int x, int y) {
#ifdef SPHERICAL_TRACKBALL
  mInitialPosition = surfaceVector(x, y);
  mInitialRotation = *mRotation;
  mInitialLightPos = *mLightPosition;
#endif
}

void Trackball::rotate(int x, int y) {
#ifdef SPHERICAL_TRACKBALL
  mCurrentPosition = surfaceVector(x, y);
  println("Model: {}, {}", x, y);

  float angle = glm::acos(glm::dot(mInitialPosition, mCurrentPosition));
  Vec3 mAxis = glm::cross(mInitialPosition, mCurrentPosition);
  mAxis = glm::normalize(mAxis);

  *mRotation = glm::rotate(mInitialRotation, angle, glm::conjugate(mInitialRotation) * mAxis);
#else
  mRotation = glm::rotate(mRotation, glm::radians(static_cast<float>(x)), glm::conjugate(mRotation) * Vec3(0, 1, 0));
  mRotation = glm::rotate(mRotation, glm::radians(static_cast<float>(y)), glm::conjugate(mRotation) * Vec3(1, 0, 0));
#endif
}

void Trackball::rotateLight(int x, int y) {
#ifdef SPHERICAL_TRACKBALL
  mCurrentPosition = surfaceVector(x, y);
  println("Light: {}, {}", x, y);

  float angle = glm::acos(glm::dot(mInitialPosition, mCurrentPosition));
  Vec3 mAxis = glm::cross(mInitialPosition, mCurrentPosition);
  mAxis = glm::normalize(mAxis);

  Quat lightRotation = glm::rotate(Quat(), angle, mAxis);
  *mLightPosition = lightRotation * mInitialLightPos;
#else
  Quat lightRotation = glm::rotate(Quat(), glm::radians(static_cast<float>(x)), Vec3(0, 1, 0));
  lightRotation = glm::rotate(lightRotation, glm::radians(static_cast<float>(y)), glm::conjugate(lightRotation) * Vec3(1, 0, 0));
  mLightPosition = lightRotation * mLightPosition;
#endif
}

void Trackball::resize(int width, int height) {
  _width = width;
  _height = height;
}
