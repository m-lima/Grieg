#include <fstream>
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

void Texture::setTexture(const std::vector<glm::vec3> &texture)
{
	init();

	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_INT, &texture);
}

void Texture::load(const std::string & name, const std::string & extension)
{
	std::ifstream file(format("assets/images/{}.{}", name, extension));
	if (!file.is_open()) {
		fatal("Couldn't open texture file {}", name, extension);
	}

	println("Loading texture: {}", name);

	mWidth = 2;
	mHeight = 2;

	std::vector<Vec3> data = { Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1), Vec3(1, 1, 1) };
	setTexture(data);
}
