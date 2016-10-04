#ifndef __INF251_TEXTURE__61287533
#define __INF251_TEXTURE__61287533

#include <SDL_image.h>
#include "infdef.hh"

struct Sampler2D {
    GLuint index;

    constexpr Sampler2D(GLuint pIndex):
        index(pIndex)
    {
    }

    Sampler2D& operator=(GLuint pIndex)
    {
        index = pIndex;
        return *this;
    }

    explicit operator GLuint() const
    {
        return index;
    }
};

class Texture;
using TexturePtr = std::shared_ptr<Texture>;

class Texture
{
    GLuint mTexture {};

    void init();

public:
    Texture() = default;

    ~Texture();

    Texture(const Texture&) = delete;

    Texture(Texture&&) = default;

    Texture& operator=(const Texture&) = delete;

    Texture& operator=(Texture&&) = default;

    void load(const std::string &name);

    void bind(Sampler2D sampler = 0) const;

    static TexturePtr cache(const std::string &name);
};
#endif //__INF251_TEXTURE__61287533
