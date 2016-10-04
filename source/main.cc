#include <fstream>
#include <sstream>
#include "Sdl.hh"
#include "Renderer.hh"

std::string readFileContents(const std::string &file)
{
    std::ifstream fh(file, std::ios::binary);
    if (!fh.is_open())
        throw std::runtime_error(format("Couldn't open file {}", file));

    std::string buf;

    fh.seekg(0, std::ios::end);
    auto length = fh.tellg();
    buf.reserve(static_cast<size_t>(length));
    fh.seekg(0, std::ios::beg);
    buf.assign(std::istreambuf_iterator<char>(fh), std::istreambuf_iterator<char>());

    return buf;
}

int main()
{
    Sdl::setGlInit(Renderer::init);
    Sdl::setGlResize(Renderer::resize);
    Sdl::setGlDisplay(Renderer::draw);
    Sdl::mainLoop();
}
