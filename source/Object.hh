#ifndef __INF251_OBJECT__68345092
#define __INF251_OBJECT__68345092

#include "infdef.hh"

class Object
{
    GLuint mBuffers[2] = {};
    GLuint mVao = 0;
    GLuint mTrigCount = 0;

    // Aliases (They don't take up any additional memory)
    GLuint& mVbo = mBuffers[0];
    GLuint& mIbo = mBuffers[1];

public:
    void load(const std::string &name);

    void setVertices(const std::vector<glm::vec3> &vertices, const std::vector<glm::uvec3> &indices);

    void bind();
    void draw();
};

#endif //__INF251_OBJECT__68345092
