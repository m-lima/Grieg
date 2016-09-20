#include "shader.hh"
#include "sdl_gl.hh"

Shader::Shader(const std::string &name)
{
    load(name);
}

Shader::~Shader()
{
    if (mProgram)
        glDeleteShader(mProgram);
}

void Shader::load(const std::string &name)
{
    GLint success = 0;

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
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCodePtr, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(fragmentShader, sizeof(log), nullptr, log);
        fmt::fatal("Couldn't compile fragment shader\nLog:\n{}", log);
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
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::bind() const
{
    glUseProgram(mProgram);
}
