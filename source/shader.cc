#include "shader.hh"
#include "sdl_gl.hh"

GLuint gUseProgram = 0;

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

Shader::UniformProxy& Shader::UniformProxy::operator=(const float f)
{
    mProgram.use();
    glUniform1f(mLoc, f);
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const std::pair<float, float> vec2)
{
    mProgram.use();
    glUniform2f(mLoc, vec2.first, vec2.second);
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Vector3 vec3)
{
    mProgram.use();
    glUniform3f(mLoc, vec3.getX(), vec3.getY(), vec3.getZ());
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Vector4 vec4)
{
    mProgram.use();
    glUniform4f(mLoc, vec4.getX(), vec4.getY(), vec4.getZ(), vec4.getW());
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Matrix3 &mat3)
{
    mProgram.use();
    glUniformMatrix3fv(mLoc, 1, GL_FALSE, reinterpret_cast<const float*>(&mat3));
    return *this;
}

Shader::UniformProxy& Shader::UniformProxy::operator=(const Matrix4 &mat4)
{
    mProgram.use();
    glUniformMatrix4fv(mLoc, 1, GL_FALSE, reinterpret_cast<const float*>(&mat4));
    return *this;
}
