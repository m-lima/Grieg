#ifndef __INF251_OBJECT__68345092
#define __INF251_OBJECT__68345092

#include "infdef.hh"

class Object
{
    // Buffers
    GLuint mBuffers[4] = {};
    GLuint mVao = 0;

    // Aliases (They don't take up any additional memory)
    GLuint& mVbo = mBuffers[0];
    GLuint& mNbo = mBuffers[2];
    GLuint& mIbo = mBuffers[1];

    GLuint mTrigCount = 0;

    void init();

public:
    ~Object();

	float scaleFactor = 1.0f;

    void load(const std::string &name);

    void setVertices(const std::vector<glm::vec3> &vertices, const std::vector<glm::ivec3> &indices);

    void setNormals(const std::vector<glm::vec3> &normals, const std::vector<glm::ivec3> &indices);

    void bind();
    void draw();
};

#endif //__INF251_OBJECT__68345092
