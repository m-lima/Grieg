#include "shader.hh"
#include "sdl_gl.hh"

GLuint gUseProgram = 0;

std::string get_type_as_string(GLenum type)
{
    switch (type) {
    case GL_FLOAT:
        return "float";

    case GL_FLOAT_VEC2:
        return "vec2 (float)";

    case GL_FLOAT_VEC3:
        return "vec3 (float)";

    case GL_FLOAT_VEC4:
        return "vec4 (float)";

    case GL_FLOAT_MAT2:
        return "mat2 (float)";

    case GL_FLOAT_MAT3:
        return "mat3 (float)";

    case GL_FLOAT_MAT4:
        return "mat4 (float)";

    default:
        return format("??? (id: {})", type);
    }
}

Shader::Shader(const std::string &name)
{
    load(name);
}

Shader::~Shader()
{
    if (mProgram)
        glDeleteProgram(mProgram);
}

void Shader::load(const std::string &name)
{
    GLint success = 0;

    println("Loading shader: {}", name);
    mName = name;

    auto vertexCode = get_file_contents(format("assets/shaders/{}.vs.glsl", name));
    auto vertexCodePtr = vertexCode.c_str();
    auto fragmentCode = get_file_contents(format("assets/shaders/{}.fs.glsl", name));
    auto fragmentCodePtr = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCodePtr, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(vertexShader, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't compile vertex shader\nLog:\n{}", log);
    } else {
        println("  vertex shader   [OK]");
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCodePtr, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(fragmentShader, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't compile fragment shader\nLog:\n{}", log);
    } else {
        println("  fragment shader [OK]");
    }

    mProgram = glCreateProgram();
    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragmentShader);
    glLinkProgram(mProgram);
    glGetProgramiv(mProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(mProgram, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't link shader program\nLog:\n{}", log);
    } else {
        println("  link program    [OK]");
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::UniformProxy Shader::uniform(const std::string &name)
{
    auto loc = glGetUniformLocation(mProgram, name.c_str());

    if (loc < 0)
        fatal("Couldn't get uniform location");

    return { *this, loc };
}

void Shader::use() const
{
    if (mProgram != gUseProgram)
    {
        gUseProgram = mProgram;
        glUseProgram(mProgram);
    }
}

void Shader::UniformProxy::assert_type(GLenum pType)
{
    char name[256];
    GLenum type;
    GLint size;

    glGetActiveUniform(mProgram.mProgram, mLoc, 256, nullptr, &size, &type, name);

    if (type != pType)
        fatal("Error assigning to uniform \"{}\" in shader \"{}\":\n  GLSL: {}\n  C++:  {}",
              name, mProgram.name(), get_type_as_string(type), get_type_as_string(pType));
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const float f)
{
    assert_type(GL_FLOAT);
    mProgram.use();
    glUniform1f(mLoc, f);
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const std::pair<float, float> vec2)
{
    assert_type(GL_FLOAT_VEC2);
    mProgram.use();
    glUniform2f(mLoc, vec2.first, vec2.second);
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Vector3 vec3)
{
    assert_type(GL_FLOAT_VEC3);
    mProgram.use();
    glUniform3f(mLoc, vec3.getX(), vec3.getY(), vec3.getZ());
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Vector4 vec4)
{
    assert_type(GL_FLOAT_VEC4);
    mProgram.use();
    glUniform4f(mLoc, vec4.getX(), vec4.getY(), vec4.getZ(), vec4.getW());
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Matrix3 &mat3)
{
    assert_type(GL_FLOAT_MAT3);
    mProgram.use();
    glUniformMatrix3fv(mLoc, 1, GL_FALSE, reinterpret_cast<const float*>(&mat3));
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Matrix4 &mat4)
{
    assert_type(GL_FLOAT_MAT4);
    mProgram.use();
    glUniformMatrix4fv(mLoc, 1, GL_FALSE, reinterpret_cast<const float*>(&mat4));
    return *this;
}
