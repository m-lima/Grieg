#ifndef __INF251_TEXT__60205946
#define __INF251_TEXT__60205946

#include "Shader.hh"
#include "Texture.hh"

class Text {
  TexturePtr mFont{};
  GLuint mVbo{};
  GLuint mIbo{};
  GLuint mVao{};
  GLsizei mTrigCount{};
  glm::vec2 mPosition{ -1.0f, 1.0f };

  void initBuffers();

  Shader _shader;

public:
  Text();

  Text(glm::vec2 position) :
    Text() {
    setPosition(position);
  }

  ~Text();

  void setFont(TexturePtr font);

  void setText(const std::string &);

  void setPosition(glm::vec2);

  template <class... Args>
  void format(fmt::CStringRef fmt, Args&&... args) {
    setText(fmt::format(fmt, std::forward<Args>(args)...));
  }

  void draw();
};

#endif //__INF251_TEXT__60205946
