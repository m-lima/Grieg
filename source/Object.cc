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
        glm::vec3 position;
        glm::vec3 normal;
    };
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

    std::vector<glm::vec3> objVertices;
    std::vector<int> vertexIndices;

    std::vector<glm::vec3> objNormals;
    std::vector<int> normalIndices;

    //std::vector<glm::vec2> texCoords;
    //std::vector<glm::uvec2> texIndices;

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

            objVertices.emplace_back(vec);
        }
        else if (tmp == "vn") {
            /* Vertex normal: vn <x:f> <y:f> <z:f> */
            glm::vec3 vec;
            nextToken(line, vec.x, left, right);
            nextToken(line, vec.y, left, right);
            nextToken(line, vec.z, left, right);
            objNormals.emplace_back(vec);
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

            vertexIndices.emplace_back(v.x);
            vertexIndices.emplace_back(v.y);
            vertexIndices.emplace_back(v.z);
            if (norm) {
                normalIndices.emplace_back(vn.x);
                normalIndices.emplace_back(vn.y);
                normalIndices.emplace_back(vn.z);
            }
        }
    }

    /* The indices start from 1 and 0, so subtract if positive.
     * If negative, then the index refers to the vertices from the end of the list
     * ie. -1 is the last vertex, -2 is next to last, and so on. */
    int vtxSize = static_cast<int>(objVertices.size());
    for (auto &idx : vertexIndices)
        idx = (idx > 0) ? idx - 1 : vtxSize + idx - 1;

    int normSize = static_cast<int>(objNormals.size());
    for (auto &idx : normalIndices)
        idx = (idx > 0) ? idx - 1 : normSize + idx - 1;

    std::map<int, std::vector<int>> vertexCache;
    std::vector<Vertex> vertexBuffer;
    std::vector<int> indexBuffer;
    for (size_t i = 0; i < normalIndices.size(); i++) {
        auto normIdx = normalIndices[i];
        auto vertIdx = vertexIndices[i];
        Vertex v;

        v.position = objVertices[vertIdx];
        v.normal = glm::normalize(objNormals[normIdx]);

        int index = -1;
        auto it = vertexCache.find(vertIdx);
        if (it == vertexCache.end()) {
            index = vertexBuffer.size();
            vertexBuffer.push_back(v);
            vertexCache.insert({ vertIdx, { 1, index } });
        } else {
            auto& vertices = it->second;
            bool found = false;

            for (const auto vtx : vertices) {
                const auto& v2 = objVertices[vtx];
                if (std::memcmp(&v2, &v, sizeof(v)) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                index = vertexBuffer.size();
                vertexBuffer.push_back(v);
                vertices.push_back(index);
            }
        }
        indexBuffer.push_back(index);
    }

    println("  positions:      {}", objVertices.size());
    println("  normals:        {}", objNormals.size());
    println("  faces:          {}", normalIndices.size());

    println("  vertices:       {}", vertexBuffer.size());
    println("  indices:        {}", indexBuffer.size());

    init();

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexBuffer.size() * sizeof(vertexBuffer[0]),
                 &vertexBuffer[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexBuffer.size() * sizeof(indexBuffer[0]),
                 &indexBuffer[0],
                 GL_STATIC_DRAW);

    mTrigCount = static_cast<GLuint>(indexBuffer.size());
}

void Object::bind()
{
    glBindVertexArray(mVao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, normal)));

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
}

void Object::draw()
{
    glDrawElements(GL_TRIANGLES, mTrigCount * 3, GL_UNSIGNED_INT, 0);
}
