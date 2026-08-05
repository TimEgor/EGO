#pragma once

#include "Texture.h"

namespace ego::gpu
{
    struct SwapChainDesc final
    {
        GraphicResourceFormat m_format = GraphicResourceFormat::Undefined;
        uint32_t m_bufferCount = 0;
    };

    class SwapChain : public GraphicObject
    {
    public:
        SwapChain(const SwapChainDesc& _desc);

        virtual Texture2DPointer getTargetTexture() = 0;

        virtual bool resize(const Texture2DSize& _size) = 0;
        virtual void present() = 0;

        const SwapChainDesc& getDesc() const;

    private:
        const SwapChainDesc m_desc;
    };

    EGO_POINTER(SwapChain);
} // namespace ego::gpu
