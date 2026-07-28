#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEngine/Graphic/SceneRender/RenderPlugin.h"

namespace ego::engine
{
    class SceneRenderController final : public NonCopyable
    {
    public:
        SceneRenderController() = default;
        ~SceneRenderController() override;

        bool init(const render::RenderPluginPointer& _renderPlugin);
        void release();
        void clearResources();

        bool prepare(const render::RenderPrepareContext& _context);
        void render(const gpu::TextureViewPointer& _targetView);
        void wait();

        render::Render& getRender();
        const render::Render& getRender() const;

        bool isInitialized() const;

    private:
        render::RenderPluginPointer m_renderPlugin = nullptr;
        render::RenderPointer m_render = nullptr;
        bool m_isFramePrepared = false;
    };
} // namespace ego::engine
