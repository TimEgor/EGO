#include "EditorSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"

ego::editor::EditorController& ego::editor::EditorSubsystem::getEditorController()
{
    return m_editorController;
}

const ego::editor::EditorController& ego::editor::EditorSubsystem::getEditorController() const
{
    return m_editorController;
}

void ego::editor::EditorSubsystem::release()
{
    m_editorController.release();
}

ego::editor::EditorSubsystemPointer ego::editor::GetEditorSubsystemPointer()
{
    return subsystem::FindSubsystem<EditorSubsystem>();
}

ego::editor::EditorSubsystem& ego::editor::GetEditorSubsystem()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_ASSERT(editorSubsystem);

    return *editorSubsystem;
}
