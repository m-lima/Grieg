#include "Texture.hh"

void Texture::init()
{
    if (!mTexture) {
        glGenTextures(1, &mTexture);
        glActiveTexture(GL_TEXTURE0 + mTextureOffset);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }
}

void Texture::load(const std::string & name)
{
    println("Loading texture: {}", name);

    auto path = format("assets/textures/{}", name);
    auto surface = IMG_Load(path.c_str());
    if (!surface)
        fatal("  Could not load texture");

    println("  width:          {}", surface->w);
    println("  height:         {}", surface->h);
    println("  bytes/px:       {}", surface->format->BitsPerPixel);

    init();

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB8,
                 surface->w,
                 surface->h,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 surface->pixels);

    SDL_FreeSurface(surface);
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, mTexture);
}
