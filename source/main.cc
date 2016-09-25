#include <fstream>
#include "sdl_gl.hh"
#include "shader.hh"
#include "trackball.hh"
#include "renderer.hh"

std::string get_file_contents(const std::string &file)
{
    std::ifstream fh(file, std::ios::binary);
    if (!fh.is_open())
        throw std::runtime_error(format("Couldn't open file {}", file));

    std::string buf;
    size_t length;

    fh.seekg(0, std::ios::end);
    length = fh.tellg();
    buf.reserve(length);
    fh.seekg(0, std::ios::beg);
    buf.assign(std::istreambuf_iterator<char>(fh), std::istreambuf_iterator<char>());

    return buf;
}

int main()
{
    Sdl sdl;
	//Trackball trackball;

    sdl.set_glinit(&Renderer::init);
    sdl.set_gldisplay(&Renderer::draw);
    sdl.main_loop();
}
