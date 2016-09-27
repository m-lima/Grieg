#include "Texture.hh"

void Texture::init()
{
	if (!mTexture) {
		glGenTextures(1, &mTexture);
		glActiveTexture(GL_TEXTURE0 + mTextureOffset);
		glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	}
}

void Texture::setTexture(SDL_Surface *surface)
{
	init();

	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_INT, surface->pixels);
}

void Texture::load(const std::string & name, const std::string & extension)
{
	IMG_Init(IMG_INIT_JPG);
	println("Loading texture: {}.{}", name, extension);
	SDL_Surface *surface = IMG_Load(format("assets / images / {}.{}", name, extension).c_str());
	if (surface == nullptr) {
		fatal("  Could not load texture");
	}

    println("  format:         {}", surface->format->format);
    println("  width:          {}", surface->w);
    println("  height:         {}", surface->h);
    println("  bytes ppx:      {}", surface->format->BitsPerPixel);

	setTexture(surface);
}
