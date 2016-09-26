#include <fstream>
#include "Object.hh"

namespace
{
    // Returns true if token was read successfully
    template<char Char = ' '>
    bool nextTokenPos(const std::string &line, size_t &left, size_t &right)
    {
        if (right == std::string::npos)
            return false;

        left = line.find_first_not_of(Char, right);
        right = line.find_first_of(Char, left);

        return line.find_first_not_of(Char, right) != std::string::npos;
    }

    template<char Char = ' '>
    bool nextToken(const std::string &line, std::string &out, size_t &left, size_t &right)
    {
        bool rv = nextTokenPos<Char>(line, left, right);
        out = line.substr(left, right - left);
        return rv;
    }

    template<char Char = ' '>
    bool nextToken(const std::string &line, float &out, size_t &left, size_t &right)
    {
        bool rv = nextTokenPos<Char>(line, left, right);
        out = std::stof(line.substr(left, right - left));
        return rv;
    }

    template<char Char = ' '>
    bool nextToken(const std::string &line, int &out, size_t &left, size_t &right)
    {
        bool rv = nextTokenPos<Char>(line, left, right);
        out = std::stoi(line.substr(left, right - left));
        return rv;
    }
}

void Object::load(const std::string &name)
{
    std::ifstream file(format("assets/meshes/{}.obj", name));
    if (!file.is_open())
        fatal("Couldn't open file {}.obj", name);

    std::vector<glm::vec3> vertices;
    std::vector<glm::uvec3> vertexIndices;

    std::vector<glm::vec3> normals;
    //std::vector<glm::uvec3> normalIndices;

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

            vertices.emplace_back(vec);
        }
        else if (tmp == "vn") {
            /* Normal vector: vn <x:f> <y:f> <z:f> */
            glm::vec3 vec;
            nextToken(line, vec.x, left, right);
            nextToken(line, vec.y, left, right);
            nextToken(line, vec.z, left, right);
            normals.emplace_back(glm::normalize(vec));
        }
        else if (tmp == "f") {
            /* Face: Kind of complicated, read the wiki */
            std::string subtok;
            size_t subleft, subright;
            glm::ivec3 v, vt, vn;

            /* Read first space-separated token, x */
            subleft = subright = 0;
            nextToken(line, subtok, left, right);
            nextToken<'/'>(subtok, v.x, subleft, subright);
            nextToken<'/'>(subtok, vt.x, subleft, subright);
            nextToken<'/'>(subtok, vn.x, subleft, subright);

            /* Read y */
            subleft = subright = 0;
            nextToken(line, subtok, left, right);
            nextToken<'/'>(subtok, v.y, subleft, subright);
            nextToken<'/'>(subtok, vt.y, subleft, subright);
            nextToken<'/'>(subtok, vn.y, subleft, subright);

            /* Read z */
            subleft = subright = 0;
            nextToken(line, subtok, left, right);
            nextToken<'/'>(subtok, v.z, subleft, subright);
            nextToken<'/'>(subtok, vt.z, subleft, subright);
            nextToken<'/'>(subtok, vn.z, subleft, subright);

            v.x--;
            v.y--;
            v.z--;

            vertexIndices.emplace_back(v);
        }
    }

    println("  vertices: {}", vertices.size());
    println("  indices: {}", vertexIndices.size());

    setVertices(vertices, vertexIndices);
}

void Object::setVertices(const std::vector<glm::vec3> &vertices, const std::vector<glm::uvec3> &indices)
{
    if (mVbo)
        glDeleteBuffers(2, mBuffers);
    glGenBuffers(2, mBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(glm::vec3),
                 reinterpret_cast<const GLfloat *>(vertices.data()),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(glm::uvec3),
                 reinterpret_cast<const GLuint *>(indices.data()),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    mTrigCount = static_cast<GLuint>(indices.size());
}

void Object::bind()
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
    glBindVertexArray(mVao);
}

void Object::draw()
{
    glDrawElements(GL_TRIANGLES, mTrigCount * 3, GL_UNSIGNED_INT, 0);
}
