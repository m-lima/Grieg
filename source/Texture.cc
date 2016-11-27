#include <unordered_map>
#include <chrono>
#include "Texture.hh"

namespace {
  std::unordered_map<std::string, std::shared_ptr<Texture>> _cache;

  using _clock = std::chrono::steady_clock;
  auto _lastUpdate = _clock::now();
}

Texture::~Texture()
{
    if (mTextures) {
        glDeleteTextures(mNumFrames, mTextures.get());
    }
}

void Texture::init(int numFrames)
{
    if (mNumFrames != numFrames) {
        if (mTextures) {
            glDeleteTextures(mNumFrames, mTextures.get());
        }

        mTextures.reset(new GLuint[numFrames]);
        glGenTextures(numFrames, mTextures.get());
        mNumFrames = numFrames;
        mFrame = 0;
    }
}

void Texture::load(const std::string & name, int numFrames)
{
    assert(numFrames > 0);

    println("Loading texture: {} with {} frames", name, numFrames);

    auto path = format("assets/textures/{}", name);
    auto surface = IMG_Load(path.c_str());
    if (!surface)
        fatal("  Could not load texture: {}", IMG_GetError());

    println("  format:         {}", SDL_GetPixelFormatName(surface->format->format));
    if (numFrames > 1) {
        println("  width:          {} ({} per frame)", surface->w, surface->w / numFrames);
    } else {
        println("  width:          {}", surface->w);
    }
    println("  height:         {}", surface->h);
    println("  bytes/px:       {}", surface->format->BitsPerPixel);

    {
        auto rgbSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        if (!rgbSurface)
            fatal("  Could not convert surface to RGBA: {}", SDL_GetError());
        SDL_FreeSurface(surface);
        surface = rgbSurface;
    }

    init(numFrames);

    auto frameHeight = surface->h / mNumFrames;
    for (int i = 0; i < mNumFrames; i++) {
        gl->glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        gl->glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, surface->w, frameHeight);
        gl->glTexSubImage2D(GL_TEXTURE_2D,
                        0, /* mipmap level */
                        0, /* x-offset */
                        0, /* y-offset */
                        surface->w,
                        frameHeight,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        reinterpret_cast<char*>(surface->pixels) + surface->pitch * frameHeight * i);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glGenerateMipmap(GL_TEXTURE_2D);
    }

    SDL_FreeSurface(surface);
}

void Texture::bind(Sampler2D sampler)
{
    if ((_clock::now() - _lastUpdate) > std::chrono::milliseconds(50)) {
        mFrame = (mFrame + 1) % mNumFrames;
        _lastUpdate = _clock::now();
    }

    gl->glActiveTexture(GL_TEXTURE0 + sampler.index);
    gl->glBindTexture(GL_TEXTURE_2D, mTextures[mFrame]);
}

std::shared_ptr<Texture> Texture::cache(const std::string &name)
{
    auto it = _cache.find(name);

    if (it != _cache.cend())
        return it->second;

    auto tex = std::make_shared<Texture>();
    tex->load(name);
    _cache[name] = tex;

    return tex;
}
