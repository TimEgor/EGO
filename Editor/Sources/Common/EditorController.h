#pragma once

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEngine/EngineSession.h"

#include "Gui/GuiController.h"
#include "ProjectController.h"

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

        bool init(const XmlDocument& _config);
        void release();

        engine::EngineSessionPointer getEditorEngineSessionPointer() const;

        GuiController& getGuiController();
        const GuiController& getGuiController() const;

        ProjectController& getProjectController();
        const ProjectController& getProjectController() const;

        PlatformSurfacePointer getMainSurfacePointer() const;
        bool isMainSurfaceValid() const;

    private:
        struct Context final
        {
            engine::EngineSessionPointer m_engineSession = nullptr;
            PlatformSurfacePointer m_mainSurface = nullptr;
        };

        bool initEditorAssets(const XmlDocument& _config);
        void releaseEditorAssets();

        bool initContext(const XmlDocument& _config);
        void releaseContext();

        Context m_editorContext;

        FileSystemPointer m_editorAssetsFileSystem = nullptr;

        GuiController m_guiController;
        ProjectController m_projectController;
    };
} // namespace ego::editor
