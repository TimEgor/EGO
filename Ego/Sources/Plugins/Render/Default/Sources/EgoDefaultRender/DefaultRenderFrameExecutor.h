#pragma once

#include <vector>

#include "EgoGraphicHardware/GraphicDevice.h"

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::render
{
    class DefaultRenderFrameExecutor final
    {
    public:
        DefaultRenderFrameExecutor() = default;

        bool init(GraphicDevice& _graphicDevice, const gpu::CommandQueueReference& _commandQueue);
        void release();
        void wait();
        void submitCommandLists(const std::vector<RenderGraphicCommandList>& _commandLists);

        bool isValid() const;

    private:
        void signalFrameFence();

        RenderCommandQueue m_commandQueue = nullptr;
        RenderFence m_frameFence = nullptr;
        gpu::Fence::FenceValue m_frameFenceValue = 0;
    };
} // namespace ego::render
