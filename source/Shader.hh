#ifndef __INF251_SHADER__21548889
#define __INF251_SHADER__21548889

#include "infdef.hh"
#include "ShaderStorage.hh"
#include "Texture.hh"

enum struct ShaderType {
  // Load shaders in the same manner as we did before
  // Vertex and fragment shader.
  custom,

  // Render meshes with respect to the camera.
  // Fragment shader.
  object,

  // Post processing. Drawn as a quad.
  // Fragment shader.
  postprocess,
};

template <class T>
struct GlslTypeinfo;

#define __GLSLASSIGN(Type, Enum, FuncSuffix, ...)                       \
    template <>                                                         \
    struct GlslTypeinfo<Type> {                                         \
        static constexpr GLenum glslEnum = Enum;                        \
        static void setUniform(GLuint program, GLuint loc, const Type &val) { \
            gl->glProgram ## FuncSuffix(program, loc, __VA_ARGS__);      \
        }                                                               \
    }

__GLSLASSIGN(Sampler2D, GL_SAMPLER_2D, Uniform1i, val.index);
__GLSLASSIGN(GLfloat, GL_FLOAT, Uniform1f, val);
__GLSLASSIGN(GLdouble, GL_DOUBLE, Uniform1d, val);
__GLSLASSIGN(GLuint, GL_UNSIGNED_INT, Uniform1i, val);
__GLSLASSIGN(GLint, GL_INT, Uniform1i, val);
__GLSLASSIGN(glm::vec2, GL_FLOAT_VEC2, Uniform2f, val.x, val.y);
__GLSLASSIGN(glm::vec3, GL_FLOAT_VEC3, Uniform3f, val.x, val.y, val.z);
__GLSLASSIGN(glm::vec4, GL_FLOAT_VEC4, Uniform4f, val.x, val.y, val.z, val.w);
__GLSLASSIGN(glm::mat2, GL_FLOAT_MAT2, UniformMatrix2fv, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&val));
__GLSLASSIGN(glm::mat3, GL_FLOAT_MAT3, UniformMatrix3fv, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&val));
__GLSLASSIGN(glm::mat4, GL_FLOAT_MAT4, UniformMatrix4fv, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&val));

#undef __GLSLASSIGN

class Shader {
    uint32_t mProgram = 0;
    std::string mName = "";

    class UniformProxy {
        const Shader& mProgram;
        const GLint mLoc;

        void assertType(GLenum type);

    public:
        UniformProxy(const Shader& program, const GLint loc):
            mProgram(program),
            mLoc(loc)
        {
        }

        template <class T>
        UniformProxy& operator=(const T &value)
        {
            using type = GlslTypeinfo<typename std::decay<T>::type>;
            if (mLoc >= 0)
            {
                assertType(type::glslEnum);
                type::setUniform(mProgram.mProgram, mLoc, value);
            }

            return *this;
        }
    };

public:
    Shader() = default;

    Shader(const std::string &name, ShaderType type = ShaderType::custom);

    Shader(const Shader &) = delete;

    Shader(Shader &&) = default;

    ~Shader();

    Shader& operator=(const Shader &) = delete;

    Shader& operator=(Shader &&) = default;

    const std::string &name() const
    {
        return mName;
    }

    void load(const std::string &name, ShaderType type = ShaderType::custom);

    UniformProxy uniform(const std::string &name);

    void use() const;

    operator bool() const
    {
        return mProgram != 0;
    }

    template <class T, size_t N>
    void bindBuffer(const ShaderStorage<T, N> &ub)
    {
        ub.bind();
        auto loc = gl->glGetProgramResourceIndex(mProgram, GL_SHADER_STORAGE_BLOCK, ub.name);
        if (loc < 0)
            fatal("Warning: Couldn't find shader storage block \"{}\" in shader \"{}\"", mName, ub.name);

        gl->glShaderStorageBlockBinding(mProgram, loc, ub.binding);
    }

    void unbindBuffer(GLuint binding)
    {
        gl->glShaderStorageBlockBinding(mProgram, 0, binding);
    }

    template <class T, size_t N>
    void unbindBuffer(const ShaderStorage<T, N> &ub)
    {
        unbindBuffer(ub.binding);
    }
};

#endif //__INF251_SHADER__21548889
