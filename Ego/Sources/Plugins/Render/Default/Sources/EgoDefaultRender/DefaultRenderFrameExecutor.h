#pragma once

#include <vector>

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicObjects/GraphicObject.h"

#include "EgoEngine/Graphic/SceneRender/RenderGpuObject.h"

namespace ego::render
{
    class DefaultRenderFrameExecutor final
    {
    public:
        DefaultRenderFrameExecutor() = default;

        bool init(GraphicDevice& _graphicDevice, const gpu::CommandQueueReference& _commandQueue);
        void release();
        void wait();
        void submitCommandLists(const std::vector<RenderGraphicCommandList>& _commandLists, std::vector<gpu::GraphicObjectReference>&& _frameResources);

        bool isValid() const;

    private:
        void signalFrameFence();

        RenderCommandQueue m_commandQueue = nullptr;
        RenderFence m_frameFence = nullptr;
        std::vector<gpu::GraphicObjectReference> m_frameResources;
        gpu::Fence::FenceValue m_frameFenceValue = 0;
    };
} // namespace ego::render
