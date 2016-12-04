#include "Camera.hh"

#include <QApplication>
#include <glm/gtc/matrix_transform.hpp>

namespace {
  int _width = 1;
  int _height = 1;
  bool _pathInitialized = false;

  glm::ivec2 _anchor;

  bool _W_down = false;
  bool _A_down = false;
  bool _S_down = false;
  bool _D_down = false;
  bool _Shift_down = false;
  bool _CTRL_down = false;
}

Camera::Camera(QWidget * parent) :
  QObject(parent),
  mFOV(45.0f),
  projectionDirty(true),
  viewDirty(true),
  lightDirty(true),
  mLightPosition{ 1.0f, 1.0f, 1.0f },
  mTranslation{ 0.0f, 0.0f, -5.0f },
  mZoomSensitivity(0.025f),
  mTranslationSensitivity(0.005f),
  trackball(&mRotation, &mLightPosition, &viewDirty),
  mParent(parent) {}

Mat4 Camera::rotation() {
  return mMode == TRACKBALL
    ? glm::translate(Mat4(mRotation), glm::conjugate(mRotation) * mTranslation)
    : Mat4(mRotation) * glm::translate(Mat4(), mTranslation);
}

Mat4 Camera::projection() {
  if (mOrtho && mMode == Camera::TRACKBALL) {
    float zoom = mFOV / 22.5f;
    float ratio = static_cast<float>(_height) / static_cast<float>(_width);
    return glm::ortho(-zoom, zoom, -zoom * ratio, zoom * ratio, 0.1f, 1000.0f);
  } else {
    return glm::perspectiveFov(
      glm::radians(mFOV),
      static_cast<float>(_width),
      static_cast<float>(_height),
      0.1f,
      1000.0f
    );
  }
}

Mat4 Camera::skyboxPV()
{
  auto proj = glm::perspectiveFov(glm::radians(mFOV),
                                  static_cast<float>(_width),
                                  static_cast<float>(_height),
                                  0.1f,
                                  1000.0f);

  auto view = rotation();
  view[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
  return proj * view;
}

Vec3 Camera::lightPosition() {
  return glm::normalize(mLightPosition);
}

Vec3 Camera::eyePosition() {
  return mMode == Camera::TRACKBALL
    ? mRotation * mTranslation
    : mTranslation;
}

void Camera::mousePressed(QMouseEvent * evt) {
  _anchor = { evt->x(), evt->y() };
  trackball.anchor(evt->x(), evt->y());
  if (mMode == Camera::WASD) {
    mParent->setCursor(Qt::BlankCursor);
  }
}

void Camera::mouseReleased(QMouseEvent * evt) {
  mParent->setCursor(Qt::ArrowCursor);
  if (mMode == Camera::WASD) {
    QCursor::setPos(mParent->mapToGlobal(QPoint(_width / 2, _height / 2)));
  }
}

void Camera::mouseMoved(QMouseEvent * evt) {
  if (evt->modifiers() & Qt::ControlModifier && mMode != Camera::WASD) {
    if (evt->buttons() & Qt::LeftButton) {
      trackball.rotateLight(evt->x(), evt->y());
      lightDirty = true;
    }
  } else {
    switch (mMode) {
      case Camera::TRACKBALL:
      default:
        if (evt->buttons() & Qt::LeftButton) {
          trackball.rotate(evt->x(), evt->y());
        } else if (evt->buttons() & Qt::RightButton) {
          translate(evt->x(), evt->y());
        }
        break;
      case Camera::WASD:
        if (evt->buttons() & Qt::LeftButton) {
          Vec2 anchor = { evt->x() - _anchor.x, evt->y() - _anchor.y };
          if (anchor.x == 0.0f && anchor.y == 0.0f) {
            return;
          }

          auto angle = glm::sqrt(anchor.x * anchor.x + anchor.y * anchor.y)
            * mTranslationSensitivity * 0.5f;
          auto axis = glm::normalize(anchor);

          QCursor::setPos(mParent->mapToGlobal(QPoint(_width / 2, _height / 2)));
          _anchor = { _width / 2, _height / 2 };

          mRotation = glm::rotate(
            mRotation,
            angle,
            glm::conjugate(mRotation) * Vec3(axis.y, axis.x, 0.0f)
          );

          viewDirty = true;
        }
        break;
      case Camera::PATH:
        break;
    }
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
      if (evt->key() == Qt::Key_Shift) {
        _Shift_down = true;
      }
      if (evt->key() == Qt::Key_Control) {
        _CTRL_down = true;
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
      if (evt->key() == Qt::Key_Shift) {
        _Shift_down = false;
      }
      if (evt->key() == Qt::Key_Control) {
        _CTRL_down = false;
      }
      break;
    case Camera::PATH:
      break;
  }
}

void Camera::setMode(Mode mode) {
  if (mode == mMode) {
    return;
  }

  if (mMode == Camera::TRACKBALL && mode == Camera::WASD) {
    mTranslation = glm::conjugate(mRotation) * mTranslation;
  } else if (mMode == Camera::WASD && mode == Camera::TRACKBALL) {
    mTranslation = Vec3(0.0f, 0.0f, -glm::distance(mTranslation, {}));
  }

  mMode = mode;
  projectionDirty = true;
  viewDirty = true;

  if (mMode == Mode::PATH && !_pathInitialized) {
    path.add({ -55.0f, -0.2f, -25.0f }, { -40.0f, -1.3f, -15.0f });
    path.add({ -40.0f, -1.3f, -15.0f }, { -30.0f, -0.2f, -16.0f });
    path.add({ -30.0f, -0.2f, -16.0f }, { -23.0f, -0.2f, -16.5f });
    path.add({ -23.0f, -0.7f, -16.5f }, { 0.0f, 0.0f, 0.0f });
    path.add({ -20.0f, -2.0f, 9.0f }, { 0.0f, 0.0f, 0.0f });
    path.add({ -5.0f, -4.0f, 25.0f }, { 0.0f, 0.0f, 0.0f });
    path.add({ 5.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
    path.add({ 0.0f, -1.0f, -5.0f }, { 0.0f, 0.0f, 0.0f });
    path.add({ -5.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
    path.add({ -22.0f, -2.0f, 5.0f }, { -55.0f, -0.2f, -25.0f });
    path.buildSplines();
    _pathInitialized = true;
  }

  stop();
}


void Camera::reset() {
  setMode(Mode::TRACKBALL);

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
  } else if (mFOV > 100.0f) {
    mFOV = 100.0f;
  }
  projectionDirty = true;
}

void Camera::togglePerspective() {
  mOrtho = !mOrtho;
  projectionDirty = true;
}

void Camera::setDefaultPosition(Position position) {
  mTranslation = Vec3(0.0f, 0.0f, -5.0f);
  mRotation = Quat();
  viewDirty = true;
  setMode(Mode::TRACKBALL);

  switch (position) {
    case Position::TOP:
      mRotation = glm::rotate(
        mRotation,
        glm::pi<float>() / 2.0f,
        Vec3(1.0f, 0.0f, 0.0f));
      break;
    case Position::BOTTOM:
      mRotation = glm::rotate(
        mRotation,
        -glm::pi<float>() / 2.0f,
        Vec3(1.0f, 0.0f, 0.0f));
      break;
    case Position::RIGHT:
      mRotation = glm::rotate(
        mRotation,
        glm::pi<float>() / 2.0f,
        Vec3(0.0f, 1.0f, 0.0f));
      break;
    case Position::LEFT:
      mRotation = glm::rotate(
        mRotation,
        -glm::pi<float>() / 2.0f,
        Vec3(0.0f, 1.0f, 0.0f));
      break;
    case Position::FRONT:
    default:
      break;
    case Position::BACK:
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
  viewDirty = true;
}

void Camera::moveTo(const Vec3 & position) {
  mTranslation = position;
  viewDirty = true;
}

void Camera::lookAt(const Vec3 & target) {
  mRotation = glm::lookAt(
    target, mTranslation, Vec3(0.0f, 1.0f, 0.0f));
  viewDirty = true;
}

void Camera::update() {
  switch (mMode) {
    case Camera::TRACKBALL:
    default:
      break;
    case Camera::WASD:
      if (_W_down
          || _A_down
          || _S_down
          || _D_down
          || _Shift_down
          || _CTRL_down) {
        Vec3 base;
        if (_W_down) {
          base += Vec3(0.0f, 0.0f, 0.1f);
        }
        if (_A_down) {
          base += Vec3(0.1f, 0.0f, 0.0f);
        }
        if (_S_down) {
          base += Vec3(0.0f, 0.0f, -0.1f);
        }
        if (_D_down) {
          base += Vec3(-0.1f, 0.0f, 0.0f);
        }
        if (_Shift_down) {
          base += Vec3(0.0f, -0.1f, 0.0f);
        }
        if (_CTRL_down) {
          base += Vec3(0.0f, 0.1f, 0.0f);
        }
        mTranslation += glm::conjugate(mRotation) * base;
        viewDirty = true;
      }
      break;
    case Camera::PATH:
      static float pathIndex = 0.0f;
      auto index = path.interp(pathIndex);
      pathIndex += 0.01f;
      moveTo(index.first);
      lookAt(index.second);
      break;
  }
}

void Camera::stop() {
  _W_down = false;
  _A_down = false;
  _S_down = false;
  _D_down = false;
  _Shift_down = false;
  _CTRL_down = false;
  mParent->setCursor(Qt::ArrowCursor);
}
