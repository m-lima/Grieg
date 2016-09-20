#ifndef __INF251_SHADER__21548889
#define __INF251_SHADER__21548889

#include "infdef.hh"

class Shader {
    uint mProgram = 0;

public:
    Shader() = default;

    Shader(const std::string &name);

    Shader(const Shader &) = delete;

    Shader(Shader &&) = default;

    ~Shader();

    Shader& operator=(const Shader &) = delete;

    Shader& operator=(Shader &&) = default;

    void load(const std::string &name);

    void bind() const;
};

#endif //__INF251_SHADER__21548889
