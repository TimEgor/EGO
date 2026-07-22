#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoECS/Entity.h"

#include "EgoGui/Rendering/GuiRenderData.h"

#include "EgoEngine/Graphic/Presenter/FramePresenterController.h"
#include "EgoEngine/Graphic/SceneRender/SceneRenderController.h"
#include "EgoEngine/Graphic/Gui/GuiRenderController.h"

namespace ego
{
    class Level;
    EGO_POINTER(Level);
} // namespace ego

namespace ego::engine
{
    class GraphicFrameController final : public NonCopyable
    {
    public:
        struct InitData final
        {
            render::RenderPluginPointer m_sceneRenderPlugin = nullptr;
            gui::GuiRenderPluginPointer m_guiRenderPlugin = nullptr;
            bool m_enablePresentation = false;
        };

        struct SceneRenderData final
        {
            LevelPointer m_activeLevel = nullptr;
            GraphicPresenterPointer m_graphicPresenter = nullptr;
            ecs::Entity m_cameraEntity;
            float m_deltaTime = 0.0f;
        };

        GraphicFrameController() = default;
        ~GraphicFrameController() override;

        bool init(const InitData& _initData);
        void release();

        void prepareFrame(gui::GuiRenderData&& _guiRenderData, const SceneRenderData& _sceneRenderData);
        void renderFrame();
        void presentFrame();

        render::Render& getRender();

    private:
        void clearResources();
        void prepareGuiFrame(gui::GuiRenderData&& _guiRenderData);
        void prepareSceneFrame(const SceneRenderData& _sceneRenderData);
        void renderGuiFrame();

        SceneRenderController m_sceneRenderController;
        GuiRenderController m_guiRenderController;
        FramePresenterController m_framePresenterController;

        std::vector<GraphicPresenterPointer> m_preparedGuiPresenters;
        GraphicPresenterPointer m_sceneGraphicPresenter = nullptr;
    };
} // namespace ego::engine
