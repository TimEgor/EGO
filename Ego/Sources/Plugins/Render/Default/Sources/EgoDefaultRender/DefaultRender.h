#pragma once

#include <vector>

#include "EgoMath/Transform.h"
#include "EgoMath/Vector.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"
#include "EgoEngine/Graphic/Render/Material.h"
#include "EgoEngine/Graphic/Render/Mesh.h"
#include "EgoEngine/Graphic/Render/Render.h"

namespace ego
{
    class DefaultRender : public Render
    {
    public:
        struct Item final
        {
            MeshHandle m_mesh = nullptr;
            MaterialHandle m_material = nullptr;
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

        virtual void setResolution(const RenderResolution& _resolution) override;
        virtual const RenderResolution& getResolution() const override;

        bool isInitialized() const;

        void setClearColor(const FloatVector4& _clearColor);
        const FloatVector4& getClearColor() const;

        void setClearEnabled(bool _enabled);
        bool isClearEnabled() const;

        EGO_RENDER(DefaultRender, Render);

    private:
        void collectRenderItems(Level& _level);
        bool prepareRenderTarget();
        bool prepareShaderData(Level& _level, ecs::Entity _cameraEntity);
        bool prepareCameraShaderData(Level& _level, ecs::Entity _cameraEntity);
        bool prepareObjectShaderData();
        void setupTargetViewport();
        void submitCommandList(const gpu::GraphicCommandListReference& _commandList);
        void transitionRenderTarget(
            const gpu::GraphicCommandListReference& _commandList,
            gpu::GraphicResourceState _nextState
        );
        bool copyRenderTargetToPresenter(GraphicPresenter& _presenter);
        void renderItem(const Item& _item);

        gpu::CommandQueueReference m_commandQueue = nullptr;
        gpu::GraphicCommandListReference m_commandList = nullptr;
        gpu::GraphicCommandListReference m_presentCommandList = nullptr;
        gpu::FenceReference m_frameFence = nullptr;
        gpu::Texture2DReference m_renderTargetTexture = nullptr;
        gpu::TextureViewReference m_renderTargetView = nullptr;
        gpu::BufferReference m_cameraShaderDataBuffer = nullptr;
        gpu::BufferViewReference m_cameraShaderDataView = nullptr;
        gpu::BufferReference m_objectShaderDataBuffer = nullptr;
        gpu::BufferViewReference m_objectShaderDataView = nullptr;

        std::vector<Item> m_renderItems;
        ComputeMatrix4x4 m_cameraViewProjectionMatrix = ComputeMatrix4x4Identity;
        RenderResolution m_resolution = DefaultRenderResolution;
        RenderResolution m_pendingResolution = DefaultRenderResolution;
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
