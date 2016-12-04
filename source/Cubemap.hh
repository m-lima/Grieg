#ifndef __INF251_CUBEMAP__4796773
#define __INF251_CUBEMAP__4796773

#include "Shader.hh"

class Cubemap {
  GLuint mTexture {};
public:
  ~Cubemap();

  Shader shader;

  void load();
  void draw();
};

#endif //__INF251_CUBEMAP__4796773
