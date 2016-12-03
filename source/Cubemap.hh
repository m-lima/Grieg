#ifndef __INF251_CUBEMAP__4796773
#define __INF251_CUBEMAP__4796773

#include "Shader.hh"

class Cubemap {
  GLuint mTexture {};
  Shader mShader;
public:
  ~Cubemap();

  template <class T, size_t N>
  void bindBuffer(const ShaderStorage<T, N> &ub)
  { mShader.bindBuffer(ub); }

  void load();
  void draw();
};

#endif //__INF251_CUBEMAP__4796773
