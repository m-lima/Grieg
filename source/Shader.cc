#include "Shader.hh"
#include "Sdl.hh"
#include "Debug.hh"

GLuint gUseProgram = 0;

Shader::Shader(const std::string &name)
{
    load(name);
}

Shader::~Shader()
{
    if (mProgram)
        gl->glDeleteProgram(mProgram);
}

void Shader::load(const std::string &name)
{
    GLint success = 0;

    println("Loading shader: {}", name);
    mName = name;

    auto vertexCode = readFileContents(format("assets/shaders/{}.vs.glsl", name));
    auto vertexCodePtr = vertexCode.c_str();
    auto fragmentCode = readFileContents(format("assets/shaders/{}.fs.glsl", name));
    auto fragmentCodePtr = fragmentCode.c_str();

    GLuint vertexShader = gl->glCreateShader(GL_VERTEX_SHADER);
    gl->glShaderSource(vertexShader, 1, &vertexCodePtr, nullptr);
    gl->glCompileShader(vertexShader);
    gl->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        gl->glGetShaderInfoLog(vertexShader, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't compile vertex shader\nLog:\n{}", log);
    } else {
        println("  vertex shader   [OK]");
    }

    GLuint fragmentShader = gl->glCreateShader(GL_FRAGMENT_SHADER);
    gl->glShaderSource(fragmentShader, 1, &fragmentCodePtr, nullptr);
    gl->glCompileShader(fragmentShader);
    gl->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        gl->glGetShaderInfoLog(fragmentShader, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't compile fragment shader\nLog:\n{}", log);
    } else {
        println("  fragment shader [OK]");
    }

    mProgram = gl->glCreateProgram();
    gl->glAttachShader(mProgram, vertexShader);
    gl->glAttachShader(mProgram, fragmentShader);
    gl->glLinkProgram(mProgram);
    gl->glGetProgramiv(mProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        gl->glGetProgramInfoLog(mProgram, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't link shader program\nLog:\n{}", log);
    } else {
        println("  link program    [OK]");
    }

    gl->glDeleteShader(vertexShader);
    gl->glDeleteShader(fragmentShader);

    gl->glBindFragDataLocation(mProgram, 0, "FragColor");
    gl->glBindFragDataLocation(mProgram, 1, "FragNormal");
}

Shader::UniformProxy Shader::uniform(const std::string &name)
{
    auto loc = gl->glGetUniformLocation(mProgram, name.c_str());

    if (loc < 0) {
        //println("Warning: Couldn't get uniform location \"{}\" in shader \"{}\"", name, mName);
        return { *this, -1 };
    }

    return { *this, loc };
}

void Shader::use() const
{
    if (mProgram != gUseProgram)
    {
        gUseProgram = mProgram;
        gl->glUseProgram(mProgram);
    }
}

void Shader::UniformProxy::assertType(GLenum pType)
{
    char name[256];
    GLenum type;
    GLint size;

    gl->glGetActiveUniform(mProgram.mProgram, mLoc, 256, nullptr, &size, &type, name);

    if (type != pType)
        fatal("Error assigning to uniform \"{}\" in shader \"{}\":\n  GLSL: {}\n  C++:  {}",
              name, mProgram.name(), Debug::GlslType(type), Debug::GlslType(pType));
}
