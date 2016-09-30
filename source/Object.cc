#include <map>
#include <fstream>
#include "Object.hh"

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
  static_assert(sizeof(Vertex) == sizeof(GLfloat) * 8);
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
    std::ifstream file(format("assets/meshes/{}.obj", name));
    if (!file.is_open())
        fatal("Couldn't open file {}.obj", name);

    struct Triangle {
        glm::ivec3 posIdx;
        glm::ivec3 texIdx;
        glm::ivec3 normIdx;

        constexpr Triangle(const glm::ivec3 pPosIdx,
                           const glm::ivec3 pTexIdx,
                           const glm::ivec3 pNormIdx):
        posIdx(pPosIdx),
            texIdx(pTexIdx),
            normIdx(pNormIdx)
        {
        }
    };

    std::vector<glm::vec3> objPositions;
    std::vector<glm::vec3> objNormals;
    std::vector<glm::vec2> objTexCoords;
    std::vector<Triangle> objTrigs;

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

        if (tmp == "v") {
            /* Vertex: v <x:f> <y:f> <z:f> [w:f] */
            glm::vec3 vec;
            nextToken(line, vec.x, left, right);
            nextToken(line, vec.y, left, right);
            nextToken(line, vec.z, left, right);

            float w;
            if (nextToken(line, w, left, right)) {
                vec /= w;
            }

            objPositions.emplace_back(vec);
        }
        else if (tmp == "vt") {
            /* Vertex texture coordinate: vt <s:f> <t:f> */
            glm::vec2 vec;
            nextToken(line, vec.s, left, right);
            nextToken(line, vec.t, left, right);
            objTexCoords.emplace_back(vec);
        }
        else if (tmp == "vn") {
            /* Vertex normal: vn <x:f> <y:f> <z:f> */
            glm::vec3 vec;
            nextToken(line, vec.x, left, right);
            nextToken(line, vec.y, left, right);
            nextToken(line, vec.z, left, right);
            objNormals.emplace_back(glm::normalize(vec));
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

            objTrigs.emplace_back(v, vt, vn);
        }
    }

    /* The indices start from 1 and 0, so subtract if positive.
     * If negative, then the index refers to the vertices from the end of the list
     * ie. -1 is the last vertex, -2 is next to last, and so on. */
    const auto posSize = static_cast<int>(objPositions.size());
    const auto texSize = static_cast<int>(objTexCoords.size());
    const auto normSize = static_cast<int>(objNormals.size());
    const auto normalizeIdx = [](int size, glm::ivec3 &v) {
        v.x = (v.x > 0) ? v.x - 1 : size + v.x - 1;
        v.y = (v.y > 0) ? v.y - 1 : size + v.y - 1;
        v.z = (v.z > 0) ? v.z - 1 : size + v.z - 1;
    };

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    for (auto &f : objTrigs) {
        normalizeIdx(posSize, f.posIdx);
        normalizeIdx(texSize, f.texIdx);
        normalizeIdx(normSize, f.normIdx);

        indices.emplace_back(vertices.size());
        vertices.emplace_back(objPositions[f.posIdx.x],
                              objTexCoords[f.texIdx.x],
                              objNormals[f.normIdx.x]);

        indices.emplace_back(vertices.size());
        vertices.emplace_back(objPositions[f.posIdx.y],
                              objTexCoords[f.texIdx.y],
                              objNormals[f.normIdx.y]);

        indices.emplace_back(vertices.size());
        vertices.emplace_back(objPositions[f.posIdx.z],
                              objTexCoords[f.texIdx.z],
                              objNormals[f.normIdx.z]);
    }

    println("  positions:      {}", objPositions.size());
    println("  texcoord:       {}", objTexCoords.size());
    println("  normals:        {}", objNormals.size());
    println("  trigs:          {}", objTrigs.size());
    println("");
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

void Object::bind()
{
    glBindVertexArray(mVao);

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

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
}

void Object::draw()
{
    glDrawElements(GL_TRIANGLES, mTrigCount * 3, GL_UNSIGNED_INT, 0);
}
