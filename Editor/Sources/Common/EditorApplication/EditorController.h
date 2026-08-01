#pragma once

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEngine/EngineSession.h"

#include "EditorApplication/EditorProjectController.h"
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

        bool init(const XmlDocument& _config);
        void release();

        engine::EngineSessionPointer getEditorEngineSessionPointer() const;

        EditorGuiController& getEditorGuiController();
        const EditorGuiController& getEditorGuiController() const;

        EditorProjectController& getProjectController();
        const EditorProjectController& getProjectController() const;

        PlatformSurfacePointer getMainSurfacePointer() const;
        bool isMainSurfaceValid() const;

    private:
        struct EditorContext final
        {
            engine::EngineSessionPointer m_engineSession = nullptr;
            PlatformSurfacePointer m_mainSurface = nullptr;
        };

        bool initEditorAssets(const XmlDocument& _config);
        void releaseEditorAssets();

        bool initEditorContext(const XmlDocument& _config);
        void releaseEditorContext();

        EditorContext m_editorContext;

        FileSystemPointer m_editorAssetsFileSystem = nullptr;

        EditorGuiController m_editorGuiController;
        EditorProjectController m_projectController;
    };
} // namespace ego::editor
