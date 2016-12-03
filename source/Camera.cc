#include "Camera.hh"

#include <glm/gtc/matrix_transform.hpp>

namespace {
  int _width = 1;
  int _height = 1;

  glm::ivec2 _anchor;

  bool _W_down = false;
  bool _A_down = false;
  bool _S_down = false;
  bool _D_down = false;

}

Camera::Camera() :
  trackball(&mRotation, &mLightPosition),
  mFOV(45.0f),
  projectionDirty(true),
  viewDirty(true),
  lightDirty(true),
  mLightPosition{ 1.0f, 1.0f, 1.0f },
  mTranslation{ 0.0f, 0.0f, -5.0f },
  mZoomSensitivity(0.025f),
  mTranslationSensitivity(0.005f) {}

Mat4 Camera::rotation() {
  return glm::translate(
    Mat4(mRotation), glm::conjugate(mRotation) * mTranslation);
}

Mat4 Camera::projection() {
  if (mOrtho) {
    float zoom = mFOV / 22.5f;
    float ratio = static_cast<float>(_height) / static_cast<float>(_width);
    return glm::ortho(-zoom, zoom, -zoom * ratio, zoom * ratio, 1.0f, 20.0f);
  } else {
    return glm::perspectiveFov(
      glm::radians(mFOV),
      static_cast<float>(_width),
      static_cast<float>(_height),
      1.0f,
      20.0f
    );
  }
}

Vec3 Camera::lightPosition() {
  return glm::normalize(mLightPosition);
}

Vec3 Camera::eyePosition() {
  return mRotation * mTranslation;
}

bool Camera::moving() {
  return _W_down | _A_down | _S_down | _D_down;
}

void Camera::mousePressed(QMouseEvent * evt) {
  switch (mMode) {
    case Camera::TRACKBALL:
    default:
      trackball.anchor(evt->x(), evt->y());
      _anchor = { evt->x(), evt->y() };
      break;
    case Camera::WASD:
      break;
    case Camera::PATH:
      break;
  }
}

void Camera::mouseMoved(QMouseEvent * evt) {
  switch (mMode) {
    case Camera::TRACKBALL:
    default:
      if (evt->modifiers() & Qt::ControlModifier) {
        if (evt->buttons() & Qt::LeftButton) {
          trackball.rotateLight(evt->x(), evt->y());
          lightDirty = true;
        }
      } else {
        if (evt->buttons() & Qt::LeftButton) {
          trackball.rotate(evt->x(), evt->y());
          viewDirty = true;
        } else if (evt->buttons() & Qt::RightButton) {
          translate(evt->x(), evt->y());
          viewDirty = true;
        }
      }
      break;
    case Camera::WASD:
      break;
    case Camera::PATH:
      break;
  }
}

void Camera::wheelMoved(QWheelEvent * evt) {
  zoom(evt->delta());
}

void Camera::keyPressed(QKeyEvent * evt) {
  switch (mMode) {
    case Camera::TRACKBALL:
    default:
      break;
    case Camera::WASD:
      if (evt->key() == Qt::Key_W) {
        _W_down = true;
      }
      if (evt->key() == Qt::Key_A) {
        _A_down = true;
      }
      if (evt->key() == Qt::Key_S) {
        _S_down = true;
      }
      if (evt->key() == Qt::Key_D) {
        _D_down = true;
      }
      break;
    case Camera::PATH:
      break;
  }
}

void Camera::keyReleased(QKeyEvent * evt) {
  switch (mMode) {
    case Camera::TRACKBALL:
    default:
      break;
    case Camera::WASD:
      if (evt->key() == Qt::Key_W) {
        _W_down = false;
      }
      if (evt->key() == Qt::Key_A) {
        _A_down = false;
      }
      if (evt->key() == Qt::Key_S) {
        _S_down = false;
      }
      if (evt->key() == Qt::Key_D) {
        _D_down = false;
      }
      break;
    case Camera::PATH:
      break;
  }
}

void Camera::reset() {
  mOrtho = false;
  mFOV = 45.0f;
  projectionDirty = true;

  mRotation = Quat();
  mTranslation = Vec3(0.0f, 0.0f, -5.0f);
  viewDirty = true;

  mLightPosition = Vec3(1.0f, 1.0f, 1.0f);
  lightDirty = true;
}

void Camera::zoom(int amount) {
  mFOV += mZoomSensitivity * amount;
  if (mFOV < 1.0f) {
    mFOV = 1.0f;
  } else if (mFOV >= 180.0f) {
    mFOV = 179.0f;
  }
  projectionDirty = true;
}

void Camera::togglePerspective() {
  mOrtho = !mOrtho;
  projectionDirty = true;
}

void Camera::setPosition(int position) {
  mTranslation = Vec3(0.0f, 0.0f, -5.0f);
  mRotation = Quat();
  viewDirty = true;

  switch (position) {
    case 0: // Top
      mRotation = glm::rotate(
        mRotation,
        glm::pi<float>() / 2.0f,
        Vec3(1.0f, 0.0f, 0.0f));
      break;
    case 1: // Bottom
      mRotation = glm::rotate(
        mRotation,
        -glm::pi<float>() / 2.0f,
        Vec3(1.0f, 0.0f, 0.0f));
      break;
    case 2: // Right
      mRotation = glm::rotate(
        mRotation,
        glm::pi<float>() / 2.0f,
        Vec3(0.0f, 1.0f, 0.0f));
      break;
    case 3: // Left
      mRotation = glm::rotate(
        mRotation,
        -glm::pi<float>() / 2.0f,
        Vec3(0.0f, 1.0f, 0.0f));
      break;
    case 4: // Front
    default:
      break;
    case 5: // Back
      mRotation = glm::rotate(
        mRotation,
        glm::pi<float>(),
        Vec3(0.0f, 1.0f, 0.0f));
      break;
  }
}

void Camera::resize(int width, int height) {
  _width = width;
  _height = height;
  projectionDirty = true;
  trackball.resize(width, height);
}

void Camera::translate(int x, int y) {
  mTranslation.x += mTranslationSensitivity * (x - _anchor.x);
  mTranslation.y -= mTranslationSensitivity * (y - _anchor.y);
  _anchor = { x, y };
}
