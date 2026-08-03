#include "GuiController.h"

#include <string>

#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/Surface/PlatformSurface.h"
#include "EgoCore/UtilsMacros.h"

#include "EditorSubsystem.h"

#include "GuiStyle.h"

ego::editor::GuiController::~GuiController()
{
    release();
}

bool ego::editor::GuiController::init(const XmlDocument& _config)
{
    release();

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_INITIALIZATION(editorSubsystem);

    EGO_CHECK_INITIALIZATION(editorSubsystem->getEditorController().getMainSurfacePointer());

    const gui::GuiControllerPointer guiController = GetGuiControllerPointer();
    EGO_CHECK_INITIALIZATION(guiController);

    FileName defaultFontPath;
    float defaultFontSize = 0.0f;
    EGO_CHECK_INITIALIZATION(readDefaultFont(_config, defaultFontPath, defaultFontSize));

    EGO_CHECK_INITIALIZATION(guiController->setFont(defaultFontPath, defaultFontSize));

    const gui::GuiStyle editorStyle = CreateGuiStyle();
    EGO_CHECK_INITIALIZATION(guiController->setStyle(editorStyle));

    EGO_CHECK_INITIALIZATION(guiController->registerLayer(*this));
    EGO_CHECK_INITIALIZATION(m_windowController.init());
    EGO_CHECK_INITIALIZATION(m_menuController.init());

    return true;
}

void ego::editor::GuiController::release()
{
    const gui::GuiControllerPointer guiController = GetGuiControllerPointer();
    if (guiController)
    {
        guiController->unregisterLayer(*this);
    }

    m_menuController.release();
    m_windowController.release();

    m_sceneTexture = nullptr;
}

void ego::editor::GuiController::setSceneTexture(const gpu::TextureViewPointer& _sceneTexture)
{
    m_sceneTexture = _sceneTexture;
}

ego::gui::GuiFrameTextureID ego::editor::GuiController::getSceneTextureID()
{
    return m_sceneTexture ? bindTexture(m_sceneTexture) : gui::InvalidGuiFrameTextureID;
}

ego::editor::GuiWindowController& ego::editor::GuiController::getWindowController()
{
    return m_windowController;
}

const ego::editor::GuiWindowController& ego::editor::GuiController::getWindowController() const
{
    return m_windowController;
}

bool ego::editor::GuiController::pushModalWindow(const GuiModalWindowPointer& _window)
{
    return m_windowController.pushModalWindow(_window);
}

ego::gui::GuiControllerPointer ego::editor::GuiController::GetGuiControllerPointer()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    const engine::EngineSessionPointer engineSession = editorSubsystem ? editorSubsystem->getEditorController().getEditorEngineSessionPointer() : nullptr;

    return engineSession ? engineSession->getGuiControllerPointer() : nullptr;
}

bool ego::editor::GuiController::readDefaultFont(const XmlDocument& _config, FileName& _path, float& _size) const
{
    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    const XmlNode fontNode = engineNode.getChild("DefaultFont");
    EGO_CHECK_RETURN_FALSE(fontNode);

    const std::string fontPath = fontNode.getChildValueOr<std::string>("Path", std::string());
    const float fontSize = fontNode.getChildValueOr<float>("Size", 0.0f);
    EGO_CHECK_RETURN_FALSE(!fontPath.empty() && fontSize > 0.0f);

    _path = FileName(fontPath);
    _size = fontSize;

    return !_path.empty();
}

void ego::editor::GuiController::drawGui()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN(editorSubsystem);

    const PlatformSurfacePointer mainSurface = editorSubsystem->getEditorController().getMainSurfacePointer();
    EGO_CHECK_RETURN(mainSurface);

    m_titleBar.draw(*mainSurface, m_menuController);

    m_windowController.drawWindows();
}
