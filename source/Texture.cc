#include <chrono>
#include <QImage>
#include "Texture.hh"

namespace {
  using _clock = std::chrono::steady_clock;
  auto _lastUpdate = _clock::now();
}

Texture::~Texture() {
  if (mTextures) {
    glDeleteTextures(mNumFrames, mTextures.get());
  }
}

void Texture::init(int numFrames) {
  if (mNumFrames != numFrames) {
    if (mTextures) {
      glDeleteTextures(mNumFrames, mTextures.get());
    }

    mTextures.reset(new GLuint[numFrames]);
    glGenTextures(numFrames, mTextures.get());
    mNumFrames = numFrames;
    mFrame = 0;
  }
}

void Texture::load(const std::string & name, int numFrames) {
  assert(numFrames > 0);

  println("Loading texture: {} with {} frames", name, numFrames);

  int index = 0;
  QImage surface;
  do {
    auto path = format(":textures{}/{}", index, name);
    surface = QImage(QString::fromStdString(path));
    index++;
  } while (index < 2 && surface.isNull());

  if (surface.isNull()) { fatal("  Could not load texture: {}", name); }

  println("  format:         {}", surface.format());
  if (numFrames > 1) {
    println("  width:          {} ({} per frame)", surface.width(), surface.width() / numFrames);
  } else {
    println("  width:          {}", surface.width());
  }
  println("  height:         {}", surface.height());

  {
    auto rgbSurface = surface.convertToFormat(QImage::Format_ARGB32);
      if (rgbSurface.isNull())
      fatal("  Could not convert surface to RGBA: {}", name);
    surface = rgbSurface;
  }

  init(numFrames);

  auto frameHeight = surface.height() / mNumFrames;
  for (int i = 0; i < mNumFrames; i++) {
    gl->glBindTexture(GL_TEXTURE_2D, mTextures[i]);
    gl->glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, surface.width(), frameHeight);
    gl->glTexSubImage2D(GL_TEXTURE_2D,
                        0, /* mipmap level */
                        0, /* x-offset */
                        0, /* y-offset */
                        surface.width(),
                        frameHeight,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        surface.bits() + surface.width() * 4 * frameHeight * i);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glGenerateMipmap(GL_TEXTURE_2D);
  }
}

void Texture::bind(Sampler2D sampler) {
  if ((_clock::now() - _lastUpdate) > std::chrono::milliseconds(50)) {
    mFrame = (mFrame + 1) % mNumFrames;
    _lastUpdate = _clock::now();
  }

  gl->glActiveTexture(GL_TEXTURE0 + sampler.index);
  gl->glBindTexture(GL_TEXTURE_2D, mTextures[mFrame]);
}
