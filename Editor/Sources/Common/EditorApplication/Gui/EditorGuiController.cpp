#include "EditorGuiController.h"

#include <string>

#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

#include "EditorApplication/EditorSubsystem.h"

#include "EditorGuiStyle.h"

ego::editor::EditorGuiController::~EditorGuiController()
{
    release();
}

bool ego::editor::EditorGuiController::init(const XmlDocument& _config)
{
    release();

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_INITIALIZATION(editorSubsystem);

    const PlatformSurfacePointer mainSurface = editorSubsystem->getEditorController().getMainSurfacePointer();
    EGO_CHECK_INITIALIZATION(mainSurface);

    const gui::GuiControllerPointer guiController = GetGuiControllerPointer();
    EGO_CHECK_INITIALIZATION(guiController);

    FileName defaultFontPath;
    float defaultFontSize = 0.0f;
    EGO_CHECK_INITIALIZATION(readDefaultFont(_config, defaultFontPath, defaultFontSize));

    EGO_CHECK_INITIALIZATION(guiController->setFont(defaultFontPath, defaultFontSize));

    const gui::GuiStyle editorStyle = CreateEditorGuiStyle();
    EGO_CHECK_INITIALIZATION(guiController->setStyle(editorStyle));

    m_mainSurface = mainSurface;
    EGO_CHECK_INITIALIZATION(guiController->registerLayer(*this));
    EGO_CHECK_INITIALIZATION(m_menuController.init());
    EGO_CHECK_INITIALIZATION(m_windowController.init());

    return true;
}

void ego::editor::EditorGuiController::release()
{
    const gui::GuiControllerPointer guiController = GetGuiControllerPointer();
    if (guiController)
    {
        guiController->unregisterLayer(*this);
    }

    m_windowController.release();
    m_menuController.release();

    m_mainSurface = nullptr;
    m_sceneTexture = nullptr;
}

void ego::editor::EditorGuiController::setSceneTexture(const gpu::TextureViewPointer& _sceneTexture)
{
    m_sceneTexture = _sceneTexture;
}

bool ego::editor::EditorGuiController::registerMenuLayer(const GuiMenuLayerPointer& _layer, GuiMenuOrder _order)
{
    return m_menuController.registerLayer(_layer, _order);
}

bool ego::editor::EditorGuiController::unregisterMenuLayer(const GuiMenuLayerPointer& _layer)
{
    return m_menuController.unregisterLayer(_layer);
}

ego::gui::GuiControllerPointer ego::editor::EditorGuiController::GetGuiControllerPointer()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    const engine::EngineSessionPointer engineSession = editorSubsystem ? editorSubsystem->getEditorController().getEditorEngineSessionPointer() : nullptr;

    return engineSession ? engineSession->getGuiControllerPointer() : nullptr;
}

bool ego::editor::EditorGuiController::readDefaultFont(const XmlDocument& _config, FileName& _path, float& _size) const
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

void ego::editor::EditorGuiController::drawGui()
{
    if (m_mainSurface)
    {
        m_titleBar.draw(*m_mainSurface, m_menuController);
    }

    gui::GuiFrameTextureID sceneTextureID = gui::InvalidGuiFrameTextureID;
    if (m_windowController.isViewportVisible() && m_sceneTexture)
    {
        sceneTextureID = bindTexture(m_sceneTexture);
    }

    m_windowController.drawWindows(sceneTextureID);
}
