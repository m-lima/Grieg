#ifndef __INF251_UNIFORMBUFFER__31298117
#define __INF251_UNIFORMBUFFER__31298117

#include "infdef.hh"

template <class Block, GLuint Binding, size_t N = 1>
class UniformBuffer {
    const char *mName;
    Block mBlock[N] {};
    GLuint mUbo {};

    void init()
    {
        if (!mUbo) {
            glGenBuffers(1, &mUbo);
            glBindBuffer(GL_UNIFORM_BUFFER, mUbo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(mBlock), mBlock, GL_DYNAMIC_DRAW);
        }
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
        return mBlock;
    }

    const Block* operator->() const
    {
        return mBlock;
    }

    Block& operator*()
    {
        return mBlock[0];
    }

    const Block& operator*() const
    {
        return mBlock[0];
    }

    Block& operator[](size_t idx)
    {
        return mBlock[idx];
    }

    const Block& operator[](size_t idx) const
    {
        return mBlock[idx];
    }

    const char* name() const
    {
        return mName;
    }

    GLuint buffer() const
    {
        return mUbo;
    }

    void bind() const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, Binding, mUbo);
    }

    constexpr GLuint binding() const
    {
        return Binding;
    }

    void update()
    {
        init();
        bind();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mBlock), mBlock);
    }
};

#endif //__INF251_UNIFORMBUFFER__31298117
