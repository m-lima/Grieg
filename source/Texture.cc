#include "Texture.hh"

void Texture::init()
{
    if (!mTexture)
        glGenTextures(1, &mTexture);
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

    glActiveTexture(GL_TEXTURE0 + mTextureOffset);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGB8, surface->w, surface->h);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0, /* mipmap level */
                    0, /* x-offset */
                    0, /* y-offset */
                    surface->w,
                    surface->h,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(surface);
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, mTexture);
}
