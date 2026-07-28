#pragma once

#include "EgoCore/Math/Vector.h"

#include "EgoEngine/Graphic/SceneRender/Render.h"

#include "DefaultRenderFileSystems.h"
#include "DefaultRenderFrameExecutor.h"
#include "DefaultRenderScene.h"
#include "DefaultRenderSettings.h"
#include "DefaultRenderShaderDataStore.h"
#include "DefaultRenderTarget.h"
#include "Passes/ClearRenderPass.h"
#include "Passes/DebugRenderPass.h"
#include "Passes/RayTracingRenderPass.h"
#include "PipelineState/RenderPipelineStateCache.h"
#include "RenderGraph/RenderPassGraph.h"

namespace ego::render
{
    class DefaultRender : public Render
    {
    public:
        DefaultRender() = default;
        ~DefaultRender() override = default;

        bool init() override;
        void release() override;
        void clearResources() override;

        bool prepare(const RenderPrepareContext& _context) override;
        void render(const gpu::TextureViewPointer& _targetView) override;
        void wait() override;

        using Render::drawPoint;
        void drawPoint(const DebugDrawPointData& _point) override;
        void drawLine(const DebugDrawLineData& _line) override;

        bool isInitialized() const;

        void setClearColor(const FloatVector4& _clearColor);
        const FloatVector4& getClearColor() const;

        void setClearEnabled(bool _enabled);
        bool isClearEnabled() const;

        EGO_RENDER(DefaultRender, Render);

    private:
        bool initPassGraph(GraphicDevice& _graphicDevice);
        void releasePassGraph();
        gpu::Texture2DPointer resolvePresentationTargetTexture(const gpu::TextureViewPointer& _targetView) const;
        void copyResultToTarget(const RenderGraphicCommandList& _commandList, const gpu::Texture2DPointer& _targetTexture);
        void handlePrepareFailure();

        DefaultRenderFileSystems m_fileSystems;
        DefaultRenderFrameExecutor m_frameExecutor;
        DefaultRenderTarget m_renderTarget;
        DefaultRenderScene m_scene;
        DefaultRenderShaderDataStore m_shaderData;
        RenderPipelineStateCache m_pipelineStateCache;
        RenderPassGraph m_passGraph;
        ClearRenderPass m_clearPass;
        RayTracingRenderPass m_rayTracingPass;
        DebugRenderPass m_debugPass;
        DefaultRenderSettings m_settings;
        bool m_isInitialized = false;
        bool m_isPrepared = false;
    };

    EGO_POINTER(DefaultRender);
} // namespace ego::render
