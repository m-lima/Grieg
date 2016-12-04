#include <QImage>
#include "Cubemap.hh"

namespace {
  enum struct Side {
    pos_x,
    neg_x,
    pos_y,
    neg_y,
    pos_z,
    neg_z,
  };

  void _loadSide(Side side)
  {
    int sidei = static_cast<int>(side);
    assert(sidei >= 0 && sidei < 6);

    const GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };

    const char *files[] = {
        "posx",
        "negx",
        "posy",
        "negy",
        "posz",
        "negz",
    };

    QImage image(QString("resources/textures/skybox_%1.jpg").arg(files[sidei]));
    if (image.isNull())
      fatal("  Could not load cubemap part: skybox_{}.tga", files[sidei]);

    image = image.convertToFormat(QImage::Format_RGB888);
    if (image.isNull())
      fatal("  Could not convert image to RGBA: {}", files[sidei]);

    gl->glTexImage2D(targets[sidei],
                     0, /* texture level */
                     GL_RGB,
                     image.width(),
                     image.height(),
                     0, /* border */
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     image.bits());
  }
}

Cubemap::~Cubemap()
{
  if (mTexture)
    gl->glDeleteTextures(1, &mTexture);
}

void Cubemap::load()
{
  println("Loading cubemap");
  gl->glDisable(GL_DEPTH_TEST);
  gl->glGenTextures(1, &mTexture);

  gl->glActiveTexture(0);
  gl->glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
  gl->glEnable(GL_DEPTH_TEST);

  _loadSide(Side::neg_x);
  _loadSide(Side::pos_x);
  _loadSide(Side::neg_y);
  _loadSide(Side::pos_y);
  _loadSide(Side::neg_z);
  _loadSide(Side::pos_z);

  gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  mShader.load("skybox");
}

void Cubemap::draw()
{
  mShader.use();
  gl->glDepthMask(GL_FALSE);
  gl->glActiveTexture(0);
  gl->glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
  gl->glDrawArrays(GL_TRIANGLES, 0, 36);
  gl->glDepthMask(GL_TRUE);
}