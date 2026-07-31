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

bool ego::editor::EditorGuiController::init(
    const XmlDocument& _config,
    const PlatformSurfacePointer& _surface,
    const TextureGraphicPresenterPointer& _simulationGraphicPresenter)
{
    release();

    EGO_CHECK_RETURN_FALSE(_surface && _simulationGraphicPresenter);

    const gui::GuiControllerPointer guiController = GetGuiControllerPointer();
    EGO_CHECK_RETURN_FALSE(guiController);

    FileName defaultFontPath;
    float defaultFontSize = 0.0f;
    EGO_CHECK_RETURN_FALSE(readDefaultFont(_config, defaultFontPath, defaultFontSize));

    EGO_CHECK_RETURN_FALSE(guiController->setFont(defaultFontPath, defaultFontSize));

    const gui::GuiStyle editorStyle = CreateEditorGuiStyle();
    EGO_CHECK_RETURN_FALSE(guiController->setStyle(editorStyle));

    const gpu::TextureViewPointer sceneTexture = _simulationGraphicPresenter->getTextureView();
    EGO_CHECK_RETURN_FALSE(sceneTexture);

    m_guiLayer.setSurface(_surface);
    m_guiLayer.setSceneTexture(sceneTexture);
    m_layerID = guiController->registerLayer(m_guiLayer);
    if (m_layerID == gui::InvalidGuiLayerID)
    {
        m_guiLayer.reset();
        return false;
    }

    return true;
}

void ego::editor::EditorGuiController::release()
{
    const gui::GuiControllerPointer guiController = GetGuiControllerPointer();
    if (guiController && m_layerID != gui::InvalidGuiLayerID)
    {
        guiController->unregisterLayer(m_layerID);
    }

    m_guiLayer.reset();
    m_layerID = gui::InvalidGuiLayerID;
}

bool ego::editor::EditorGuiController::isInitialized() const
{
    return GetGuiControllerPointer() && m_layerID != gui::InvalidGuiLayerID;
}

ego::gui::GuiControllerPointer ego::editor::EditorGuiController::GetGuiControllerPointer()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    return editorSubsystem ? editorSubsystem->getEditorController().getGuiControllerPointer() : nullptr;
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
