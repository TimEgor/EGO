#pragma once

#include <cstdint>
#include <vector>

#include "EgoMath/Transform.h"
#include "EgoMath/Vector.h"

#include "EgoEngine/Platform/FileSystem/RootedFileSystem.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"
#include "EgoEngine/Graphic/Render/Render.h"

#include "DebugRender/DebugRender.h"

namespace ego::render
{
    class DefaultRender : public Render
    {
    public:
        struct Item final
        {
            RenderMesh m_mesh = nullptr;
            RenderMaterial m_material = nullptr;
            Transform m_globalTransform;
            uint32_t m_objectIndex = 0;
        };

        DefaultRender() = default;
        ~DefaultRender() override = default;

        virtual bool init() override;
        virtual void release() override;
        virtual void clearResources() override;

        virtual bool prepare(Level& _level, ecs::Entity _cameraEntity) override;
        virtual void render() override;
        virtual void wait() override;
        virtual void present(GraphicPresenter& _presenter) override;

        virtual void setResolution(const gpu::Texture2DSize& _resolution) override;
        virtual const gpu::Texture2DSize& getResolution() const override;

        virtual RenderGraphicPipeline createPipeline(
            const RenderVertexShader& _vertexShader,
            const RenderPixelShader& _pixelShader
        ) override;

        using Render::drawPoint;
        using Render::drawLine;
        virtual void drawPoint(const DebugDrawPointData& _point) override;
        virtual void drawLine(const DebugDrawLineData& _line) override;

        bool isInitialized() const;

        void setClearColor(const FloatVector4& _clearColor);
        const FloatVector4& getClearColor() const;

        void setClearEnabled(bool _enabled);
        bool isClearEnabled() const;

        EGO_RENDER(DefaultRender, Render);

    private:
        bool initPluginFileSystem();
        void releasePluginFileSystem();
        void collectRenderItems(Level& _level);
        bool prepareRenderTarget();
        bool prepareShaderData(Level& _level, ecs::Entity _cameraEntity);
        bool prepareCameraShaderData(Level& _level, ecs::Entity _cameraEntity);
        bool prepareObjectShaderData();
        void setupTargetViewport();
        void submitCommandList(const RenderGraphicCommandList& _commandList);
        void transitionRenderTarget(
            const RenderGraphicCommandList& _commandList,
            gpu::GraphicResourceState _nextState
        );
        bool copyRenderTargetToPresenter(GraphicPresenter& _presenter);
        void renderItem(const Item& _item);

        DefaultRenderDebugDraw m_debugDraw;
        RootedFileSystemPointer m_pluginFileSystem = nullptr;
        RenderCommandQueue m_commandQueue = nullptr;
        RenderGraphicCommandList m_commandList = nullptr;
        RenderGraphicCommandList m_presentCommandList = nullptr;
        RenderFence m_frameFence = nullptr;
        RenderTexture2D m_renderTargetTexture = nullptr;
        RenderTextureView m_renderTargetView = nullptr;
        RenderBuffer m_cameraShaderDataBuffer = nullptr;
        RenderBufferView m_cameraShaderDataView = nullptr;
        RenderBuffer m_objectShaderDataBuffer = nullptr;
        RenderBufferView m_objectShaderDataView = nullptr;
        RenderBindingLayout m_bindingLayout = nullptr;

        std::vector<Item> m_renderItems;
        ComputeMatrix4x4 m_cameraViewProjectionMatrix = ComputeMatrix4x4Identity;
        gpu::Texture2DSize m_resolution = DefaultRenderResolution;
        gpu::Texture2DSize m_pendingResolution = DefaultRenderResolution;
        FloatVector4 m_clearColor = FloatVector4(0.0f, 0.0f, 0.0f, 1.0f);
        gpu::GraphicResourceState m_renderTargetState = gpu::GraphicResourceState::Common;
        gpu::Fence::FenceValue m_frameFenceValue = 0;
        uint32_t m_objectShaderDataCapacity = 0;
        bool m_isInitialized = false;
        bool m_isPrepared = false;
        bool m_clearEnabled = true;
    };

    EGO_POINTER(DefaultRender);
}
