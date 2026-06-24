#pragma once

#include <vector>

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

namespace ego::render
{
    class DefaultRenderFrameExecutor final
    {
    public:
        DefaultRenderFrameExecutor() = default;

        bool init(GraphicDevice& _graphicDevice, const gpu::CommandQueueReference& _commandQueue);
        void release();
        void wait();
        void submitCommandList(const RenderGraphicCommandList& _commandList);
        void submitCommandLists(const std::vector<RenderGraphicCommandList>& _commandLists);

        bool isValid() const;
        const RenderGraphicCommandList& getPresentCommandList() const;

    private:
        void signalFrameFence();

        RenderCommandQueue m_commandQueue = nullptr;
        RenderGraphicCommandList m_presentCommandList = nullptr;
        RenderFence m_frameFence = nullptr;
        gpu::Fence::FenceValue m_frameFenceValue = 0;
    };
} // namespace ego::render
