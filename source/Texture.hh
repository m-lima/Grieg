#ifndef __INF251_TEXTURE__61287533
#define __INF251_TEXTURE__61287533

#include <SDL_image.h>
#include "infdef.hh"

class Texture
{
    //static unsigned int mTextureCount;

    GLuint mTexture = 0;
    GLuint mTextureOffset;

    void init();

public:
    Texture() : mTextureOffset(0) {};//mTextureCount++) {};

    Texture(Texture&&) = default;

    Texture& operator=(Texture&&) = default;

    void load(const std::string &name);

    void bind() const;

    GLuint texture() const { return mTextureOffset; };
};
#endif //__INF251_TEXTURE__61287533
