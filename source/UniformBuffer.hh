#ifndef __INF251_UNIFORMBUFFER__31298117
#define __INF251_UNIFORMBUFFER__31298117

#include "infdef.hh"

template <class Block>
class UniformBuffer {
    const char *mName;
    Block mBlock {};
    GLuint mUbo {};

    void init()
    {
        if (!mUbo)
            glGenBuffers(1, &mUbo);
    }

    void bind(GLuint index = 0)
    {
        init();
        glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, index, mUbo);
    }

public:
    UniformBuffer(const char *name):
        mName(name)
    {
    }

    UniformBuffer(const UniformBuffer&) = delete;

    UniformBuffer(UniformBuffer&&) = default;

    UniformBuffer(const char *name, const Block &block):
        mName(name),
        mBlock(block)
    {
        update();
    }

    UniformBuffer(Block&& block):
        mBlock(std::move(block))
    {
        init();
    }

    ~UniformBuffer()
    {
        if (mUbo)
            glDeleteBuffers(1, &mUbo);
    }

    UniformBuffer& operator=(const UniformBuffer&) = delete;

    UniformBuffer& operator=(UniformBuffer&&) = default;

    UniformBuffer& operator=(const Block& block)
    {
        mBlock = block;
        update();
        return *this;
    }

    UniformBuffer& operator=(Block&& block)
    {
        mBlock = std::move(block);
        update();
        return *this;
    }

    Block* operator->()
    {
        return &mBlock;
    }

    const Block* operator->() const
    {
        return &mBlock;
    }

    Block& operator*()
    {
        return mBlock;
    }

    const Block& operator*() const
    {
        return mBlock;
    }

    const char* name() const
    {
        return mName;
    }

    GLuint buffer() const
    {
        return mUbo;
    }

    void update()
    {
        bind();
        glBufferData(GL_UNIFORM_BUFFER, sizeof(mBlock), &mBlock, GL_DYNAMIC_DRAW);
    }
};

#endif //__INF251_UNIFORMBUFFER__31298117
