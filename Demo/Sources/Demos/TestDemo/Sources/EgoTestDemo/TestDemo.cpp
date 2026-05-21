#include "TestDemo.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Platform/Platform.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"

namespace
{
    bool ReadAssetsRootPath(const ego::FileName& _configPath, ego::FileName& _assetsRootPath)
    {
        ego::XmlDocument configDocument;
        EGO_CHECK_RETURN_FALSE(configDocument.loadFromFile(_configPath));

        const ego::XmlNode rootNode = configDocument
                                      .getRootNode()
                                      .getChild("CommonConfig")
                                      .getChild("Assets")
                                      .getChild("AssetsPath")
                                      .getChild("RootPath");

        const char* assetsRootPath = rootNode.getValue().get<const char*>();
        EGO_CHECK_RETURN_FALSE(assetsRootPath && assetsRootPath[0]);

        _assetsRootPath = assetsRootPath;
        return true;
    }
}

ego::demo::TestDemo::TestDemo(const FileName& _configPath)
    : m_configPath(_configPath)
{}

bool ego::demo::TestDemo::init()
{
    FileName assetsRootPath;
    EGO_CHECK_RETURN_FALSE(ReadAssetsRootPath(m_configPath, assetsRootPath));

    engine::Engine& engine = engine::GetEngine();
    ResourceController& resourceController = engine.getResourceController();

    m_assetsFileSystem = new RootedFileSystem(engine.getPlatform().getFileSystem(), assetsRootPath);
    EGO_CHECK_INITIALIZATION(m_assetsFileSystem && m_assetsFileSystem->init());

    resourceController.addFileSystem(m_assetsFileSystem);

    m_textResource = resourceController.load<TextResource>("TestDemoText.txt");
    EGO_CHECK_INITIALIZATION(m_textResource && m_textResource->isLoaded());

    m_loadedText = m_textResource->getText();

    m_triangleMesh = resourceController.load<MeshResource>("TestTriangle.mesh.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMesh && m_triangleMesh->isLoaded());

    m_triangleMaterial = resourceController.load<MaterialResource>("TestTriangle.material.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMaterial && m_triangleMaterial->isLoaded());

    engine.getRender().addRenderItem(m_triangleMesh->getMesh(), m_triangleMaterial->getMaterial());
    return true;
}

void ego::demo::TestDemo::release()
{
    const engine::EnginePointer engine = engine::EngineCore::GetInstance().getEngine();
    if (engine)
    {
        engine->getRender().clearRenderItems();
    }

    m_triangleMaterial = nullptr;
    m_triangleMesh = nullptr;

    m_loadedText.clear();
    m_textResource = nullptr;

    if (m_assetsFileSystem)
    {
        if (engine)
        {
            engine->getResourceController().removeFileSystem(m_assetsFileSystem);
        }

        EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_assetsFileSystem);
    }
}

const std::string& ego::demo::TestDemo::getLoadedText() const
{
    return m_loadedText;
}
