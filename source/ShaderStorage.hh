#ifndef __INF251_SHADERSTORAGE__31298117
#define __INF251_SHADERSTORAGE__31298117

#include "infdef.hh"

template <class Block, size_t = 1>
class ShaderStorage {
    Block mBlock {};
    GLuint mSsbo {};

    void init()
    {
        if (!mSsbo) {
            gl.glGenBuffers(1, &mSsbo);
            gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsbo);
            gl.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mBlock), &mBlock, GL_DYNAMIC_DRAW);
        }
    }

public:
    static constexpr auto name = Block::name;
    static constexpr auto binding = Block::binding;

    ShaderStorage() = default;

    ShaderStorage(const ShaderStorage&) = delete;

    ShaderStorage(ShaderStorage&&) = default;

    ShaderStorage(const Block &block):
        mBlock(block)
    {
        update();
    }

    ShaderStorage(Block&& block):
        mBlock(std::move(block))
    {
        update();
    }

    ~ShaderStorage()
    {
        if (mSsbo)
            gl.glDeleteBuffers(1, &mSsbo);
    }

    ShaderStorage& operator=(const ShaderStorage&) = delete;

    ShaderStorage& operator=(ShaderStorage&&) = default;

    ShaderStorage& operator=(const Block& block)
    {
        mBlock = block;
        update();
        return *this;
    }

    ShaderStorage& operator=(Block&& block)
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

    GLuint buffer() const
    {
        return mSsbo;
    }

    void bind() const
    {
        gl.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, mSsbo);
    }

    void update()
    {
        init();
        bind();
        gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(mBlock), &mBlock);
    }
};

template <class Block, size_t N>
class ShaderStorage<Block[], N> {
    Block mBlock[N] {};
    GLuint mSsbo {};

    void init()
    {
        if (!mSsbo) {
            gl.glGenBuffers(1, &mSsbo);
            gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsbo);
            gl.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mBlock), mBlock, GL_DYNAMIC_DRAW);
        }
    }

public:
    static constexpr auto name = Block::name;
    static constexpr auto binding = Block::binding;

    ShaderStorage() = default;

    ShaderStorage(const ShaderStorage&) = delete;

    ShaderStorage(ShaderStorage&&) = default;

    ~ShaderStorage()
    {
        if (mSsbo)
            gl.glDeleteBuffers(1, &mSsbo);
    }

    ShaderStorage& operator=(const ShaderStorage&) = delete;

    ShaderStorage& operator=(ShaderStorage&&) = default;

    Block& operator[](size_t idx)
    {
        return mBlock[idx];
    }

    const Block& operator[](size_t idx) const
    {
        return mBlock[idx];
    }

    GLuint buffer() const
    {
        return mSsbo;
    }

    void bind() const
    {
        gl.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, mSsbo);
    }

    void update()
    {
        init();
        bind();
        gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(mBlock), mBlock);
    }
};
#endif //__INF251_SHADERSTORAGE__31298117
