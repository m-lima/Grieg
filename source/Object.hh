#ifndef __INF251_OBJECT__68345092
#define __INF251_OBJECT__68345092

#include "Texture.hh"
#include "Shader.hh"
#include "infdef.hh"

class Object
{
    struct MaterialGroup {
        size_t count;
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Texture> bump;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    // Buffers
    GLuint mVao = 0;
    GLuint mVbo = 0;
    GLuint mIbo = 0;

    GLuint mTrigCount = 0;

    glm::vec3 mPosition {};

    std::vector<MaterialGroup> mMaterialGroups;

    std::shared_ptr<Shader> mShader {};

    void loadObjFile(const std::string &name);
    void loadBinFile(const std::string &name);
    void init();

public:
    Object() = default;
    ~Object();

    glm::mat4 modelTransform;

    bool enableTexture = true;

    void load(const std::string &name);

    void setShader(std::shared_ptr<Shader> shader) {
        mShader = shader;
    }

    void setMaterial(std::shared_ptr<Texture> texture, glm::vec3 ambient = { 0.0f, 0.0f, 0.0f }, glm::vec3 diffuse = { 0.5f, 0.5f, 0.5f }, glm::vec3 specular = { 0.3f, 0.3f, 0.3f }) {
        mMaterialGroups.clear();
        mMaterialGroups.push_back({mTrigCount, texture, nullptr, ambient, diffuse, specular});
    }

    void setBump(std::shared_ptr<Texture> bump, glm::vec3 ambient = { 0.0f, 0.0f, 0.0f }, glm::vec3 diffuse = { 0.5f, 0.5f, 0.5f }, glm::vec3 specular = { 0.3f, 0.3f, 0.3f }) {
      mMaterialGroups.clear();
      mMaterialGroups.push_back({ mTrigCount, nullptr, bump, ambient, diffuse, specular });
    }

    void setPosition(glm::vec3 position) {
        mPosition = position;
    }

    void update();
    void bind();
    void draw();
};

#endif //__INF251_OBJECT__68345092
