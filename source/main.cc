#include <fstream>
#include <sstream>
#include "Sdl.hh"
#include "Renderer.hh"

#ifdef _WIN32
// Force high performance GPU
extern "C" {
  // NVidia
  __declspec(dllexport) DWORD NvOptimusEnablement = 1;

  // AMD
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

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
