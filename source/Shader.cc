#include "Shader.hh"
#include "Debug.hh"

#include <QFile>
#include <QTextStream>

namespace {
  auto _objectVertexShader =
    "#version 430\n"

    "layout(location = 0) in vec3 vPosition;"
    "layout(location = 1) in vec2 vTexCoord;"
    "layout(location = 2) in vec3 vNormal;"
    "out vec3 fPosition;"
    "out vec2 fTexCoord;"
    "out vec3 fNormal;"
    "out vec3 fEyePos;"

    "layout(std430, binding = 0) buffer MatrixBlock {"
    "  mat4 uProj;"
    "  mat4 uView;"
    "};"

    "uniform mat4 uModel;"

    "void main() {"
    "  vec4 vmp = uModel * vec4(vPosition, 1.0);"
    "  fPosition = vmp.xyz;"
    "  gl_Position = uProj * uView * vmp;"
    "  fTexCoord = vTexCoord;"
    "  fNormal = normalize((uModel * vec4(normalize(vNormal), 1.0)).xyz);"
    "  fEyePos = (inverse(uView) * inverse(uModel) * vec4(0.0, 0.0, 5.0, 1.0)).xyz;"
    "}";

  auto _postprocessVertexShader =
    "#version 430\n"

    // Just define the quad as a GLSL constant so as to avoid uploading it to
    // the GPU with a VBO
    "const vec2 vtx[4] = { vec2(1.0, 1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(-1.0, -1.0) };"

    "void main() {"
    "  gl_Position = vec4(vtx[gl_VertexID], 0.0, 1.0);"
    "}";

  // We Java now
  class ShaderBuilder {
    std::vector<GLuint> mShaders;

  public:
    ~ShaderBuilder();

    enum Type {
      vertex,
      tesselation,
      fragment,
    };

    void addFile(const std::string &name, Type type, const std::string &prepend = "");

    void add(const std::string &code, Type type);

    GLuint build();
  };

  ShaderBuilder::~ShaderBuilder()
  {
    for (auto shader : mShaders)
      gl->glDeleteShader(shader);
  }

  void ShaderBuilder::addFile(const std::string &name, Type type, const std::string &prepend)
  {
    const char *typeToSuffix[] = {
        "vs", // vertex
        "ts", // tesselation
        "fs", // fragment
    };

    QString filename = QString(":shaders/%1.%2.glsl")
        .arg(QString::fromStdString(name), typeToSuffix[type]);
    QFile codeFile(filename);
    codeFile.open(QFile::ReadOnly | QFile::Text);
    QTextStream codeStream(&codeFile);
    QString code = codeStream.readAll();
    code.prepend(QString::fromStdString(prepend));
    add(code.toStdString(), type);
  }

  void ShaderBuilder::add(const std::string &code, Type type)
  {
    GLenum typeToGLenum[] = {
        GL_VERTEX_SHADER,
        0,
        GL_FRAGMENT_SHADER,
    };

    const char *typeToPretty[] = {
        "vertex",
        "tesselation",
        "fragment",
    };

    auto codePtr = code.data();
    auto codePtrPtr = &codePtr;
    auto shader = gl->glCreateShader(typeToGLenum[static_cast<int>(type)]);
    gl->glShaderSource(shader, 1, codePtrPtr, nullptr);
    gl->glCompileShader(shader);

    GLint success{};
    gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char log[1024];
      gl->glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
      fmt::fatal("Coudldn't compile {} shader\nLog:\n{}", typeToPretty[type], log);
    }

    mShaders.emplace_back(shader);
  }

  GLuint ShaderBuilder::build()
  {
    GLuint program = gl->glCreateProgram();

    for (auto shader : mShaders)
      gl->glAttachShader(program, shader);

    GLint success{};
    gl->glLinkProgram(program);
    gl->glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      char log[1024];
      gl->glGetProgramInfoLog(program, sizeof(log), nullptr, log);
      fmt::fatal("Couldn't link shader program\nLog:\n{}", log);
    }

    for (auto shader : mShaders)
      gl->glDeleteShader(shader);

    mShaders.clear();

    return program;
  }
}

Shader::Shader(const std::string &name, ShaderType type)
{
  load(name, type);
}

Shader::~Shader()
{
  if (mProgram)
    gl->glDeleteProgram(mProgram);
}

void Shader::load(const std::string &name, ShaderType type)
{
  GLint success = 0;

  println("Loading shader: {}", name);
  mName = name;

  ShaderBuilder builder;

  switch (type) {
  case ShaderType::custom:
    builder.addFile(name, ShaderBuilder::vertex);
    builder.addFile(name, ShaderBuilder::fragment);
    break;

  case ShaderType::object:
    builder.add(_objectVertexShader, ShaderBuilder::vertex);
    builder.addFile(name, ShaderBuilder::fragment);
    break;

  case ShaderType::postprocess:
    builder.add(_postprocessVertexShader, ShaderBuilder::vertex);
    builder.addFile(name, ShaderBuilder::fragment);
    break;
  }
  mProgram = builder.build();

  gl->glBindFragDataLocation(mProgram, 0, "FragColor");
  gl->glBindFragDataLocation(mProgram, 1, "FragNormal");
  //gl->glBindFragDataLocation(mProgram, 2, "FragDepth");
}

Shader::UniformProxy Shader::uniform(const std::string &name)
{
  auto loc = gl->glGetUniformLocation(mProgram, name.c_str());

  if (loc < 0) {
    //println("Warning: Couldn't get uniform location \"{}\" in shader \"{}\"", name, mName);
    return {*this, -1};
  }

  return {*this, loc};
}

void Shader::use() const
{
  gl->glUseProgram(mProgram);
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
