#include "EditorSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"

bool ego::editor::EditorSubsystem::init(const application::ApplicationPointer& _application, const engine::EnginePointer& _engine, const XmlDocument& _config)
{
    return m_editorController.init(_application, _engine, _config);
}

void ego::editor::EditorSubsystem::release()
{
    m_editorController.release();
}

void ego::editor::EditorSubsystem::update()
{
    m_editorController.update();
}

bool ego::editor::EditorSubsystem::isInitialized() const
{
    return m_editorController.isInitialized();
}

bool ego::editor::EditorSubsystem::isSurfaceValid() const
{
    return m_editorController.isSurfaceValid();
}

ego::editor::EditorController& ego::editor::EditorSubsystem::getEditorController()
{
    return m_editorController;
}

const ego::editor::EditorController& ego::editor::EditorSubsystem::getEditorController() const
{
    return m_editorController;
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
