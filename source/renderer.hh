#ifndef __INF251_RENDERER__48721384
#define __INF251_RENDERER__48721384

#include "sdl_gl.hh"

namespace Renderer {
	void loadVertices();
	void loadProgram();
	void checkAndLoadUniforms();

	void init(int width, int height);
	void draw(Update);
};

#endif //__INF251_RENDERER__48721384