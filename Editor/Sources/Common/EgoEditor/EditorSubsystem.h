#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "EgoEditor/EditorController.h"

namespace ego::editor
{
    class EditorSubsystem final : public subsystem::Subsystem
    {
    public:
        EditorSubsystem() = default;
        ~EditorSubsystem() override = default;

        EditorController& getEditorController();
        const EditorController& getEditorController() const;

        EGO_SUBSYSTEM(EditorSubsystem, subsystem::Subsystem);

    private:
        void release() override;

        EditorController m_editorController;
    };

    EGO_POINTER(EditorSubsystem);

    EditorSubsystemPointer GetEditorSubsystemPointer();
    EditorSubsystem& GetEditorSubsystem();
} // namespace ego::editor
