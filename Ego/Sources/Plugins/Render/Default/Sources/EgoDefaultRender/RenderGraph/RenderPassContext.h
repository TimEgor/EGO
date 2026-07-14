#pragma once

#include "EgoDefaultRender/DefaultRenderScene.h"
#include "EgoDefaultRender/DefaultRenderSettings.h"
#include "EgoDefaultRender/DefaultRenderShaderDataStore.h"
#include "EgoDefaultRender/DefaultRenderTarget.h"

namespace ego::gui
{
    class GuiController;
} // namespace ego::gui

namespace ego::render
{
    class RenderPipelineStateCache;

    struct RenderPassInitContext final
    {
        GraphicDevice& m_graphicDevice;
        gpu::GraphicResourceFormat m_renderTargetFormat = gpu::GraphicResourceFormat::Undefined;
    };

    struct RenderPassPrepareContext final
    {
        GraphicDevice& m_graphicDevice;
        DefaultRenderTarget& m_renderTarget;
        DefaultRenderScene& m_scene;
        DefaultRenderShaderDataStore& m_shaderData;
        const DefaultRenderSettings& m_settings;
        gui::GuiController& m_guiController;
        float m_deltaTime = 0.0f;
    };

    struct RenderPassExecuteContext final
    {
        GraphicDevice& m_graphicDevice;
        RenderPipelineStateCache& m_pipelineStateCache;
        const RenderGraphicCommandList& m_commandList;
        DefaultRenderTarget& m_renderTarget;
        DefaultRenderScene& m_scene;
        DefaultRenderShaderDataStore& m_shaderData;
        const DefaultRenderSettings& m_settings;
    };
} // namespace ego::render
