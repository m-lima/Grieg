#include <map>
#include <fstream>
#include "Object.hh"
#include <glm/gtc/matrix_transform.hpp>

namespace
{
  // Returns true if token was read successfully
  template<char Char = ' ', bool Skip = true>
  bool nextTokenPos(const std::string &line, size_t &left, size_t &right)
  {
      if (right == std::string::npos)
          return false;

      left = Skip ? line.find_first_not_of(Char, right) : (right == 0 ? right : right + 1);
      right = line.find_first_of(Char, left);

      return true;
  }

  template<char Char = ' ', bool Skip = true>
  bool nextToken(const std::string &line, std::string &out, size_t &left, size_t &right)
  {
      if (nextTokenPos<Char, Skip>(line, left, right)) {
          out = line.substr(left, right - left);
          return !out.empty();
      }
      return false;
  }

  template<char Char = ' ', bool Skip = true>
  bool nextToken(const std::string &line, float &out, size_t &left, size_t &right)
  {
      std::string tok;
      if (nextToken<Char, Skip>(line, tok, left, right)) {
          out = std::stod(tok);
          return true;
      }
      return false;
  }

  template<char Char = ' ', bool Skip = true>
  bool nextToken(const std::string &line, int &out, size_t &left, size_t &right)
  {
      std::string tok;
      if (nextToken<Char, Skip>(line, tok, left, right)) {
          out = std::stoi(tok);
          return true;
      }
      return false;
  }

  struct Vertex
  {
      glm::vec3 pos;
      glm::vec2 tex;
      glm::vec3 norm;

      constexpr Vertex(const glm::vec3 &pPos,
                       const glm::vec2 &pTex,
                       const glm::vec3 &pNorm):
      pos(pPos),
          tex(pTex),
          norm(pNorm)
      {
      }
  };
  //static_assert(sizeof(Vertex) == sizeof(GLfloat) * 8);

  struct ObjFile {
      struct Material {
          std::string name = "";
          size_t count = 0;
      };

      struct Triangle {
          size_t matIdx;
          glm::ivec3 posIdx;
          glm::ivec3 texIdx;
          glm::ivec3 normIdx;
      };

      std::string materialLib { };
      std::vector<Material> materials { 1 };
      std::vector<glm::vec3> positions { };
      std::vector<glm::vec3> normals { };
      std::vector<glm::vec2> texCoords { };
      std::vector<Triangle> trigs { };
  };

  ObjFile readObjFile(const std::string &name) {
      ObjFile obj;

      std::ifstream file(format("assets/meshes/{}.obj", name));
      if (!file.is_open())
          fatal("Couldn't open file {}.obj", name);

      println("Loading mesh: {}", name);

      while (!file.eof()) {
          std::string line, tmp;
          std::getline(file, line);

          if (line.empty() || line[0] == '#')
              continue;

          size_t left, right = 0;
          nextToken(line, tmp, left, right);

          /*
           * <x:f>: x is a required float
           * [w:i]: w is an optional integer
           */

          if (tmp == "mtllib") {
              nextToken(line, obj.materialLib, left, right);
          }
          else if (tmp == "usemtl") {
              ObjFile::Material mat;
              nextToken(line, mat.name, left, right);
              obj.materials.push_back(mat);
          }
          else if (tmp == "v") {
              /* Vertex: v <x:f> <y:f> <z:f> [w:f] */
              glm::vec3 vec;
              nextToken(line, vec.x, left, right);
              nextToken(line, vec.y, left, right);
              nextToken(line, vec.z, left, right);

              float w;
              if (nextToken(line, w, left, right)) {
                  vec /= w;
              }

              obj.positions.emplace_back(vec);
          }
          else if (tmp == "vt") {
              /* Vertex texture coordinate: vt <s:f> <t:f> */
              glm::vec2 vec;
              nextToken(line, vec.s, left, right);
              nextToken(line, vec.t, left, right);
              vec.t = 1.0f - vec.t;
              obj.texCoords.emplace_back(vec);
          }
          else if (tmp == "vn") {
              /* Vertex normal: vn <x:f> <y:f> <z:f> */
              glm::vec3 vec;
              nextToken(line, vec.x, left, right);
              nextToken(line, vec.y, left, right);
              nextToken(line, vec.z, left, right);
              obj.normals.emplace_back(glm::normalize(vec));
          }
          else if (tmp == "f") {
              /* Face: Kind of complicated, read the wiki */
              std::string subtok;
              size_t subleft, subright;
              glm::ivec3 v, vt, vn;
              bool tex = false;
              bool norm = false;

              /* Read first space-separated token, x */
              subleft = subright = 0;
              nextToken(line, subtok, left, right);
              nextToken<'/', false>(subtok, v.x, subleft, subright);
              if (nextToken<'/', false>(subtok, vt.x, subleft, subright)) tex = true;
              if (nextToken<'/', false>(subtok, vn.x, subleft, subright)) norm = true;

              /* Read y */
              subleft = subright = 0;
              nextToken(line, subtok, left, right);
              nextToken<'/', false>(subtok, v.y, subleft, subright);
              nextToken<'/', false>(subtok, vt.y, subleft, subright);
              nextToken<'/', false>(subtok, vn.y, subleft, subright);

              /* Read z */
              subleft = subright = 0;
              nextToken(line, subtok, left, right);
              nextToken<'/', false>(subtok, v.z, subleft, subright);
              nextToken<'/', false>(subtok, vt.z, subleft, subright);
              nextToken<'/', false>(subtok, vn.z, subleft, subright);

              obj.trigs.push_back({obj.materials.size() - 1, v, vt, vn});
              obj.materials.back().count++;
          }
      }

      return obj;
  }

  struct MtlFile {
      struct Material {
          glm::vec3 ambient {};
          glm::vec3 diffuse {};
          glm::vec3 specular {};
          std::shared_ptr<Texture> texture {};
      };

      std::map<std::string, Material> materials;
  };

  MtlFile readMtlFile(const std::string &name) {
      MtlFile mtl;

      std::ifstream file(format("assets/meshes/{}", name));
      if (!file.is_open())
          fatal("Couldn't open material file {}", name);

      println("Loading materials: {}", name);

      MtlFile::Material *mat = nullptr;

      while (!file.eof()) {
          std::string line, tmp;
          std::getline(file, line);

          if (line.empty() || line[0] == '#')
              continue;

          size_t left, right = 0;
          nextToken(line, tmp, left, right);

          if (tmp == "newmtl") {
              std::string name;
              nextToken(line, name, left, right);
              mat = &mtl.materials[name];
          } else if (tmp == "Ka") {
              auto& vec = mat->ambient;
              nextToken(line, vec.r, left, right);
              nextToken(line, vec.g, left, right);
              nextToken(line, vec.b, left, right);
          } else if (tmp == "Kd") {
              auto& vec = mat->diffuse;
              nextToken(line, vec.r, left, right);
              nextToken(line, vec.g, left, right);
              nextToken(line, vec.b, left, right);
          } else if (tmp == "Ks") {
              auto& vec = mat->specular;
              nextToken(line, vec.r, left, right);
              nextToken(line, vec.g, left, right);
              nextToken(line, vec.b, left, right);
          } else if (tmp == "map_Kd") {
              std::string str;
              nextToken(line, str, left, right);
              mat->texture = std::make_shared<Texture>();
              mat->texture->load(str);
          }
      }

      return mtl;
  }
}

Object::~Object()
{
    if (!mVbo)
        glDeleteBuffers(1, &mVbo);

    if (!mIbo)
        glDeleteBuffers(1, &mIbo);

    if (mVao)
        glDeleteVertexArrays(1, &mVao);
}

void Object::init()
{
    if (!mVbo)
        glGenBuffers(1, &mVbo);

    if (!mIbo)
        glGenBuffers(1, &mIbo);

    if (!mVao)
        glGenVertexArrays(1, &mVao);
}

void Object::load(const std::string &name)
{
    auto obj = readObjFile(name);
    auto mtl = !obj.materialLib.empty() ? readMtlFile(obj.materialLib) : MtlFile {};

    /* The indices start from 1 and 0, so subtract if positive.
     * If negative, then the index refers to the vertices from the end of the list
     * ie. -1 is the last vertex, -2 is next to last, and so on. */
    const auto posSize = static_cast<int>(obj.positions.size());
    const int texSize = static_cast<int>(obj.texCoords.size());
    const int normSize = static_cast<int>(obj.normals.size());
    const auto normalizeIdx = [](int size, glm::ivec3 &v) {
        v.x = (v.x > 0) ? v.x - 1 : size + v.x - 1;
        v.y = (v.y > 0) ? v.y - 1 : size + v.y - 1;
        v.z = (v.z > 0) ? v.z - 1 : size + v.z - 1;
    };
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    MaterialGroup *mat = nullptr;
    size_t matIdx = 0;
    for (auto &f : obj.trigs) {
        normalizeIdx(posSize, f.posIdx);
        normalizeIdx(texSize, f.texIdx);
        normalizeIdx(normSize, f.normIdx);

        indices.emplace_back(vertices.size());
        vertices.emplace_back(obj.positions[f.posIdx.x],
                              obj.texCoords[f.texIdx.x],
                              obj.normals[f.normIdx.x]);

        indices.emplace_back(vertices.size());
        vertices.emplace_back(obj.positions[f.posIdx.y],
                              obj.texCoords[f.texIdx.y],
                              obj.normals[f.normIdx.y]);

        indices.emplace_back(vertices.size());
        vertices.emplace_back(obj.positions[f.posIdx.z],
                              obj.texCoords[f.texIdx.z],
                              obj.normals[f.normIdx.z]);

        if (!mat || f.matIdx != matIdx) {
            auto&& objMat = obj.materials[matIdx];
            if (obj.materialLib.empty()) {
                mMaterialGroups.push_back({objMat.count, nullptr});
            } else {
                mMaterialGroups.push_back({objMat.count, mtl.materials[objMat.name].texture});
            }
            matIdx = f.matIdx;
            mat = &mMaterialGroups.back();
        }
    }

    println("  positions:      {}", obj.positions.size());
    println("  texcoords:      {}", obj.texCoords.size());
    println("  normals:        {}", obj.normals.size());
    println("  faces:          {}", obj.trigs.size());
    println("  materials:      {}", mMaterialGroups.size());
    println("  vertices:       {}", vertices.size());
    println("  indices:        {}", indices.size());

    init();

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(vertices[0]),
                 &vertices[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(indices[0]),
                 &indices[0],
                 GL_STATIC_DRAW);

    mTrigCount = static_cast<GLuint>(indices.size());
}

void Object::update()
{
    glm::mat4 mat {};
    mat = glm::translate(mat, mPosition);
    mat = mat * modelTransform;

    mShader->uniform("uModel") = mat;
    mShader->uniform("uHaveTexture") = static_cast<int>(haveTexture);
}

void Object::bind()
{
    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_TRUE,
                          sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, pos)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_TRUE,
                          sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, tex)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          3,
                          GL_FLOAT,
                          GL_TRUE,
                          sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, norm)));
}

void Object::draw()
{
    update();
    mShader->use();
    bind();
    const GLuint *start = nullptr;
    for (const auto &mat : mMaterialGroups) {
        if (mat.texture)
            mat.texture->bind();
        auto count = mat.count;

        if (mMaterialGroups.size() == 1)
            count = mTrigCount;

        glDrawElements(GL_TRIANGLES, count * 3, GL_UNSIGNED_INT, start);
        start += count * 3;
    }
}
