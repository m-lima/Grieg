#ifndef __INF251_SHADER__21548889
#define __INF251_SHADER__21548889

#include "infdef.hh"
#include <glad/glad.h>
#include "vmath.hpp"
using namespace Vectormath::Aos;

std::string get_type_as_string(GLenum);

class Shader {
    uint32_t mProgram = 0;
    std::string mName = "";

    class UniformProxy {
        const Shader& mProgram;
        const GLint mLoc;

    public:
        UniformProxy(const Shader& program, const GLint loc):
                mProgram(program),
                mLoc(loc)
        {
        }

        void assert_type(GLenum type);

        UniformProxy& operator=(const float f);

        UniformProxy& operator=(const std::pair<float, float> vec2);

        UniformProxy& operator=(const Vector3 vec3);

        UniformProxy& operator=(const Vector4 vec4);

        UniformProxy& operator=(const Matrix3 &mat3);

        UniformProxy& operator=(const Matrix4 &mat4);
    };

public:
    Shader() = default;

    Shader(const std::string &name);

    Shader(const Shader &) = delete;

    Shader(Shader &&) = default;

    ~Shader();

    Shader& operator=(const Shader &) = delete;

    Shader& operator=(Shader &&) = default;

    const std::string &name() const
    {
        return mName;
    }

    void load(const std::string &name);

    UniformProxy uniform(const std::string &name);

    void use() const;
};

#endif //__INF251_SHADER__21548889
