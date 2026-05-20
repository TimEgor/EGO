#pragma once

#include <vector>

#include "EgoMath/Vector.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"
#include "EgoEngine/Graphic/Render/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/Render/Resources/MeshResource.h"

#include "Render.h"

namespace ego
{
    struct DefaultRenderItem final
    {
        MeshResourcePointer m_mesh = nullptr;
        MaterialResourcePointer m_material = nullptr;
    };

    class DefaultRender : public Render
    {
    public:
        DefaultRender() = default;
        ~DefaultRender() override = default;

        virtual bool init() override;
        virtual void release() override;

        virtual void render(GraphicPresenter& _presenter) override;
        virtual void wait() override;
        virtual void present(GraphicPresenter& _presenter) override;

        bool isInitialized() const;

        void setClearColor(const FloatVector4& _clearColor);
        const FloatVector4& getClearColor() const;

        void setClearEnabled(bool _enabled);
        bool isClearEnabled() const;

        void addRenderItem(const MeshResourcePointer& _mesh, const MaterialResourcePointer& _material);
        void clearRenderItems();
        const std::vector<DefaultRenderItem>& getRenderItems() const;

        EGO_RENDER(DefaultRender, Render);

    private:
        bool prepareRenderTarget(const gpu::Texture2DPointer& _targetTexture);
        void setupTargetViewport(const gpu::Texture2DPointer& _targetTexture);
        void renderItem(const DefaultRenderItem& _item);

        gpu::CommandQueuePointer m_commandQueue = nullptr;
        gpu::GraphicCommandListPointer m_commandList = nullptr;
        gpu::Texture2DPointer m_renderTargetTexture = nullptr;
        gpu::TextureViewPointer m_renderTargetView = nullptr;

        std::vector<DefaultRenderItem> m_renderItems;
        FloatVector4 m_clearColor = FloatVector4(0.0f, 0.0f, 0.0f, 1.0f);
        bool m_isInitialized = false;
        bool m_clearEnabled = true;
    };

    EGO_POINTER(DefaultRender);
}
