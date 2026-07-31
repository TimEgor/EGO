#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "EditorController.h"

namespace ego::editor
{
    class EditorSubsystem final : public subsystem::Subsystem
    {
    public:
        EditorSubsystem() = default;
        ~EditorSubsystem() override = default;

        bool init(const application::ApplicationPointer& _application, const engine::EnginePointer& _engine, const XmlDocument& _config);
        void release() override;

        void update();

        bool isInitialized() const;
        bool isSurfaceValid() const;

        EditorController& getEditorController();
        const EditorController& getEditorController() const;

        EGO_SUBSYSTEM(EditorSubsystem, subsystem::Subsystem);

    private:
        EditorController m_editorController;
    };

    EGO_POINTER(EditorSubsystem);

    EditorSubsystemPointer GetEditorSubsystemPointer();
    EditorSubsystem& GetEditorSubsystem();
} // namespace ego::editor
