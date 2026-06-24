#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

namespace ego::render
{
    class RenderDeviceContext final
    {
    public:
        RenderDeviceContext() = default;

        bool init(GraphicDevice& _graphicDevice);
        void release();

        bool isValid() const;
        const gpu::CommandQueueReference& getGraphicCommandQueue() const;

    private:
        gpu::CommandQueueReference m_graphicCommandQueue = nullptr;
    };
} // namespace ego::render
