#pragma once

#include <vector>

#include "EgoMath/Transform.h"
#include "EgoMath/Vector.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"
#include "EgoEngine/Graphic/Render/Material.h"
#include "EgoEngine/Graphic/Render/Mesh.h"

#include "Render.h"

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
        void clearResources();

        virtual void render(GraphicPresenter& _presenter, Level& _level, ecs::Entity _cameraEntity) override;
        virtual void wait() override;
        virtual void present(GraphicPresenter& _presenter) override;

        bool isInitialized() const;

        void setClearColor(const FloatVector4& _clearColor);
        const FloatVector4& getClearColor() const;

        void setClearEnabled(bool _enabled);
        bool isClearEnabled() const;

        EGO_RENDER(DefaultRender, Render);

    private:
        void collectRenderItems(Level& _level);
        bool prepareRenderTarget(const gpu::Texture2DReference& _targetTexture);
        bool prepareShaderData(Level& _level, ecs::Entity _cameraEntity);
        bool prepareCameraShaderData(Level& _level, ecs::Entity _cameraEntity);
        bool prepareObjectShaderData();
        void setupTargetViewport(const gpu::Texture2DReference& _targetTexture);
        void renderItem(const Item& _item);

        gpu::CommandQueueReference m_commandQueue = nullptr;
        gpu::GraphicCommandListReference m_commandList = nullptr;
        gpu::Texture2DReference m_renderTargetTexture = nullptr;
        gpu::TextureViewReference m_renderTargetView = nullptr;
        gpu::BufferReference m_cameraShaderDataBuffer = nullptr;
        gpu::BufferViewReference m_cameraShaderDataView = nullptr;
        gpu::BufferReference m_objectShaderDataBuffer = nullptr;
        gpu::BufferViewReference m_objectShaderDataView = nullptr;

        std::vector<Item> m_renderItems;
        ComputeMatrix4x4 m_cameraViewProjectionMatrix = ComputeMatrix4x4Identity;
        FloatVector4 m_clearColor = FloatVector4(0.0f, 0.0f, 0.0f, 1.0f);
        uint32_t m_objectShaderDataCapacity = 0;
        bool m_isInitialized = false;
        bool m_clearEnabled = true;
    };

    EGO_POINTER(DefaultRender);
}
