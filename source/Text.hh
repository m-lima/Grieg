#ifndef __INF251_TEXT__60205946
#define __INF251_TEXT__60205946

#include "Texture.hh"

class Text {
    TexturePtr mFont {};
    GLuint mVbo {};
    GLuint mIbo {};
    GLuint mVao {};
    GLsizei mTrigCount {};
    glm::vec2 mPosition { -1.0f, 1.0f };

    void initBuffers();

public:
    Text(bool registerGlobally = true);

    Text(glm::vec2 position, bool registerGlobally = true):
        Text(registerGlobally)
    {
        setPosition(position);
    }

    ~Text();

    void setFont(TexturePtr font);

    void setText(const std::string &);

    void setPosition(glm::vec2);

    template <class... Args>
    void format(fmt::CStringRef fmt, Args&&... args)
    {
        setText(fmt::format(fmt, std::forward<Args>(args)...));
    }

    void draw();

    static void setGlobalFont(TexturePtr font);

    static void drawAll();
};

#endif //__INF251_TEXT__60205946
