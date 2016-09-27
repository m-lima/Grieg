#ifndef __INF251_TEXTURE__61287533
#define __INF251_TEXTURE__61287533

#include <SDL_image.h>
#include "infdef.hh"

class Texture
{
	//static unsigned int mTextureCount;

	GLuint mTexture = 0;
	const GLuint mTextureOffset;

	void init();
	void setTexture(SDL_Surface *surface);
	
public:
	Texture() : mTextureOffset(0) {};//mTextureCount++) {};

	void load(const std::string &name, const std::string &extension);
	GLuint texture() const { return mTextureOffset; };
};
#endif __INF251_TEXTURE__61287533