#ifndef __INF251_OBJECT__68345092
#define __INF251_OBJECT__68345092

#include "infdef.hh"

class Object
{
    struct MaterialGroup {
        size_t count;

        MaterialGroup() = default;

        MaterialGroup(size_t pCount):
            count(pCount)
        {
        }
    };

    // Buffers
    GLuint mVao = 0;
    GLuint mVbo = 0;
    GLuint mIbo = 0;

    GLuint mTrigCount = 0;

    std::vector<MaterialGroup> mMaterialGroups;

    void init();

public:
    Object() : modelDirty(true) {};
    ~Object();

    bool modelDirty : 1;
    Mat4 modelTransform;

    void load(const std::string &name);

    void bind();
    void draw();
};

#endif //__INF251_OBJECT__68345092
