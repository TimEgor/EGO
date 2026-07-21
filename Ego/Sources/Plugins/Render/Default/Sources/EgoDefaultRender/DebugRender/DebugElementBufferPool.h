#pragma once

#include <cstdint>
#include <vector>

#include "EgoEngine/Graphic/SceneRender/RenderGpuObject.h"

namespace ego::render
{
    class DebugElementBufferPool final
    {
    public:
        struct BufferDesc final
        {
            uint32_t m_stride = 0;
            gpu::GraphicResourceUsage m_usage = gpu::GraphicResourceUsageUndefined;
            gpu::CommonGraphicResourceAccess m_access = gpu::GraphicResourceAccessUndefined;
        };

        DebugElementBufferPool() = default;
        ~DebugElementBufferPool();

        bool init(const BufferDesc& _bufferDesc, uint32_t _elementCountPerBuffer);
        void release();

        bool updateBuffers(float _deltaTime, uint32_t _requiredElementCount);
        void updateBuffers(float _deltaTime);

        bool prepareSpace(uint32_t _requiredElementCount);
        void freeUnusedSpace();

        uint32_t getElementCountPerBuffer() const
        {
            return m_elementCountPerBuffer;
        }
        uint32_t getBufferCount() const;
        const RenderBuffer& getBuffer(uint32_t _bufferIndex) const;

        const gpu::BufferDesc& getBufferDesc() const
        {
            return m_bufferDesc;
        }

    private:
        struct BufferData final
        {
            RenderBuffer m_buffer = nullptr;
            float m_unusedTime = 0.0f;
        };

        using BufferCollection = std::vector<BufferData>;

        RenderBuffer createNewBuffer() const;
        void addDelayTime(float _deltaTime);
        bool activateBuffers(uint32_t _requiredBufferCount);
        void deactivateBuffers(uint32_t _requiredBufferCount);

        static uint32_t CalculateRequiredBufferCount(uint32_t _requiredElementCount, uint32_t _elementCountPerBuffer);

        BufferCollection m_buffers;
        gpu::BufferDesc m_bufferDesc;
        uint32_t m_elementCountPerBuffer = 0;
        uint32_t m_activeBufferCount = 0;

        static constexpr float BufferReleasingDelay = 2.0f;
    };

    class DebugElementBufferPoolIterator final
    {
    public:
        DebugElementBufferPoolIterator(const DebugElementBufferPool& _bufferPool, bool _iterateOnInit = true);
        ~DebugElementBufferPoolIterator();

        void* getCurrentElement() const
        {
            return m_currentElement;
        }

        template <typename T>
        T* getCurrentTypedElement() const
        {
            return static_cast<T*>(m_currentElement);
        }

        bool isInInitialState() const;
        bool next();

    private:
        bool mapCurrentBuffer();
        void unmapCurrentBuffer();

        const DebugElementBufferPool& m_bufferPool;
        RenderBuffer m_currentBuffer = nullptr;
        void* m_currentBufferData = nullptr;
        void* m_currentElement = nullptr;
        uint32_t m_currentBufferIndex = 0;
        uint32_t m_currentElementIndex = 0;
        bool m_iterationStarted = false;
    };
} // namespace ego::render
