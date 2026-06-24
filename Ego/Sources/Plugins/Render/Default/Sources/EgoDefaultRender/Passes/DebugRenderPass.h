#pragma once

#include "EgoDefaultRender/DebugRender/DebugRender.h"
#include "EgoDefaultRender/RenderGraph/RenderPass.h"

namespace ego::render
{
    class DebugRenderPass final : public RenderPass
    {
    public:
        DebugRenderPass() = default;

        bool init(RenderPassInitContext& _context) override;
        void release() override;
        void clearResources() override;
        void declare(RenderPassBuilder& _builder) override;
        bool prepare(RenderPassPrepareContext& _context) override;
        void execute(RenderPassExecuteContext& _context) override;

        void drawPoint(const DebugDrawPointData& _point);
        void drawLine(const DebugDrawLineData& _line);

    private:
        static bool LoadDebugDrawInitData(DefaultRenderDebugDraw::InitData& _initData);
        static void SetupTargetViewport(RenderPassExecuteContext& _context);

        RenderBindingLayout m_bindingLayout = nullptr;
        DefaultRenderDebugDraw m_debugDraw;
    };
} // namespace ego::render
