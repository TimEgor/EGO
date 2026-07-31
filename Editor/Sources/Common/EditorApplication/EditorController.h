#pragma once

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/Presentation/TextureGraphicPresenter.h"

#include "EgoGui/Gui.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Level/Level.h"

#include "EgoApplication/Application.h"

#include "EditorApplication/Gui/EditorGuiController.h"

namespace ego
{
    class FileSystem;

    EGO_POINTER(FileSystem);
} // namespace ego

namespace ego::editor
{
    class EditorController final : public NonCopyable
    {
    public:
        EditorController() = default;
        ~EditorController() override;

        bool init(const application::ApplicationPointer& _application, const engine::EnginePointer& _engine, const XmlDocument& _config);
        void release();

        void update();

        bool isInitialized() const;
        bool isSurfaceValid() const;
        gui::GuiControllerPointer getGuiControllerPointer() const;

    private:
        struct EditorContext final
        {
            engine::EngineSessionPointer m_engineSession = nullptr;
            PlatformSurfacePointer m_surface = nullptr;
        };

        struct ProjectContext final
        {
            engine::EngineSessionPointer m_simulationSession = nullptr;
            TextureGraphicPresenterPointer m_simulationGraphicPresenter = nullptr;
            LevelPointer m_simulationLevel = nullptr;
        };

        bool initEditorAssets(const XmlDocument& _config);
        void releaseEditorAssets();
         
        bool initEditorContext(const XmlDocument& _config);
        void releaseEditorContext();

        bool initProjectContext(const XmlDocument& _config);
        void releaseProjectContext();
        void drawProjectContext();

        application::ApplicationPointer m_application = nullptr;
        engine::EnginePointer m_engine = nullptr;

        EditorContext m_editorContext;
        ProjectContext m_projectContext;

        FileSystemPointer m_editorAssetsFileSystem = nullptr;

        EditorGuiController m_editorGuiController;
    };
} // namespace ego::editor
