#include <string>
#include <set>
#include "Shader.hh"
#include "Text.hh"

constexpr glm::vec2 logicalSize = { 2.0f / 64.0f, 2.0f / 48.0f };
constexpr glm::vec2 topLeft = { -1.0f, 1.0f };

namespace {
  TexturePtr _gFont;
  std::set<Text*> _texts;

  Shader _shader;

  struct TextVertex {
      glm::vec2 pos;
      glm::vec2 tex;
  };
}

Text::Text(bool registerGlobally):
    mFont(_gFont)
{
    if (registerGlobally)
        _texts.insert(this);
}

Text::~Text()
{
    _texts.erase(this);

    if (mVbo)
        gl->glDeleteBuffers(1, &mVbo);

    if (mIbo)
        gl->glDeleteBuffers(1, &mIbo);

    if (mVao)
        gl->glDeleteVertexArrays(1, &mVao);
}

void Text::initBuffers()
{
    if (!_shader) {
        _shader.load("text");
        _shader.uniform("uFont") = Sampler2D(0);
    }

    if (!mVbo)
        gl->glGenBuffers(1, &mVbo);

    if (!mIbo)
        gl->glGenBuffers(1, &mIbo);

    if (!mVao)
        gl->glGenVertexArrays(1, &mVao);
}

void Text::setFont(TexturePtr font)
{
    mFont = font ? font : _gFont;
}

void Text::setText(const std::string &str)
{
    initBuffers();

    constexpr float dx = 1.0f / 16.0f;
    constexpr float dy = 1.0f / 16.0f;

    std::vector<TextVertex> vertices;
    std::vector<GLuint> indices;

    auto pos = mPosition;
    glm::vec2 color = { 1.0f, 1.0f };
    for (size_t i = 0; i < str.size(); i++) {
        char c = str.at(i);

        if (c == '\n') {
            pos.y -= logicalSize.y;
            pos.x = mPosition.x;
            continue;
        } else if (c == ' ') {
            pos.x += logicalSize.x;
            continue;
        }

        float charx = (c % 16) * dx;
        float chary = (c / 16) * dy;
        size_t idxOff = vertices.size();

        vertices.push_back({{pos.x, pos.y}, {charx, chary}});
        vertices.push_back({{pos.x + logicalSize.x, pos.y}, {charx + dx, chary}});
        vertices.push_back({{pos.x + logicalSize.x, pos.y - logicalSize.y}, {charx + dx, chary + dy}});
        vertices.push_back({{pos.x, pos.y - logicalSize.y}, {charx, chary + dy}});

        indices.push_back(idxOff);
        indices.push_back(idxOff+1);
        indices.push_back(idxOff+2);
        indices.push_back(idxOff);
        indices.push_back(idxOff+3);
        indices.push_back(idxOff+2);

        pos.x += logicalSize.x;
    }


    gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);

    gl->glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(vertices[0]),
                 &vertices[0],
                 GL_DYNAMIC_DRAW);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(indices[0]),
                 &indices[0],
                 GL_DYNAMIC_DRAW);
    mTrigCount = indices.size() / 3;
}

void Text::setPosition(glm::vec2 position) {
    position.y = -position.y;
    mPosition = position * logicalSize + topLeft;
}

void Text::draw()
{
    if (!mFont || !mVbo || !mTrigCount)
        return;

    mFont->bind();
    _shader.use();

    gl->glBindVertexArray(mVao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);

    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0,
                          2,
                          GL_FLOAT,
                          GL_TRUE,
                          sizeof(TextVertex),
                          reinterpret_cast<const void*>(offsetof(TextVertex, pos)));

    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_TRUE,
                          sizeof(TextVertex),
                          reinterpret_cast<const void*>(offsetof(TextVertex, tex)));

    gl->glDrawElements(GL_TRIANGLES, mTrigCount * 3, GL_UNSIGNED_INT, nullptr);

    gl->glDisableVertexAttribArray(1);
    gl->glDisableVertexAttribArray(0);
}

void Text::setGlobalFont(TexturePtr font)
{
    assert(font);
    _gFont = font;

    for (auto&& text : _texts)
        if (!text->mFont)
            text->mFont = font;
}

void Text::drawAll()
{
    for (auto&& text : _texts)
        text->draw();
}
