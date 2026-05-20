#pragma once

#include <cstdint>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Buffer.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego
{
    struct VertexBufferBinding final
    {
        gpu::BufferPointer m_buffer = nullptr;
        uint32_t m_stride = 0;
        uint32_t m_offset = 0;
    };

    class MeshResource final : public Resource
    {
    public:
        MeshResource() = default;

        const VertexBufferBinding& getVertexBuffer() const;
        void setVertexBuffer(const gpu::BufferPointer& _buffer, uint32_t _stride, uint32_t _offset = 0);

        const gpu::BufferPointer& getIndexBuffer() const;
        gpu::GraphicResourceFormat getIndexFormat() const;
        uint32_t getIndexOffset() const;
        void setIndexBuffer(
            const gpu::BufferPointer& _buffer,
            gpu::GraphicResourceFormat _format,
            uint32_t _offset = 0
        );

        uint32_t getVertexCount() const;
        void setVertexCount(uint32_t _count);

        uint32_t getIndexCount() const;
        void setIndexCount(uint32_t _count);

        EGO_RESOURCE(MeshResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        VertexBufferBinding m_vertexBuffer;
        gpu::BufferPointer m_indexBuffer = nullptr;
        gpu::GraphicResourceFormat m_indexFormat = gpu::GraphicResourceFormat::Undefined;
        uint32_t m_indexOffset = 0;
        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;
    };

    EGO_POINTER(MeshResource);
}
