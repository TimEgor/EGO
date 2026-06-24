#pragma once

#include "EgoDefaultRender/RenderGraph/RenderPass.h"

namespace ego::render
{
    class ClearRenderPass final : public RenderPass
    {
    public:
        ClearRenderPass() = default;

        bool init(RenderPassInitContext& _context) override;
        void release() override;
        void clearResources() override;
        void declare(RenderPassBuilder& _builder) override;
        bool prepare(RenderPassPrepareContext& _context) override;
        void execute(RenderPassExecuteContext& _context) override;
    };
} // namespace ego::render
