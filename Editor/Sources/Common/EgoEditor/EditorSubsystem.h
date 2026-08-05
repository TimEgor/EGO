#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "EgoEditor/EditorController.h"

namespace ego::editor
{
    class EditorSubsystem final : public subsystem::Subsystem
    {
    public:
        EditorSubsystem() = default;
        ~EditorSubsystem() override;

        EditorController& getEditorController();
        const EditorController& getEditorController() const;

        EGO_SUBSYSTEM(EditorSubsystem, subsystem::Subsystem);

    private:
        void onUnregistered() override;
        void release();

        EditorController m_editorController;
    };

    EGO_POINTER(EditorSubsystem);

    EditorSubsystemPointer GetEditorSubsystemPointer();
    EditorSubsystem& GetEditorSubsystem();
} // namespace ego::editor
