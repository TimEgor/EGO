#include "EgoGui/GuiController.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/GeneralResources/BinaryResource.h"
#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoGui/Implementation/GuiBackendFactory.h"

ego::gui::GuiController::GuiController()
    : m_backend(CreateGuiBackend())
{
}

ego::gui::GuiController::~GuiController()
{
    release();
}

bool ego::gui::GuiController::init(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(m_backend && !m_backend->isInitialized());

    return m_backend->init(_initData.m_viewportProvider, _initData.m_enableMultiViewport);
}

void ego::gui::GuiController::release()
{
    EGO_ASSERT(!m_isFrameActive);
    if (m_isFrameActive)
    {
        return;
    }

    EGO_ASSERT(m_layers.empty());
    m_layers.clear();
    m_pendingFrame = GuiRenderData();
}

bool ego::gui::GuiController::setFont(const FileName& _path, float _size)
{
    EGO_CHECK_RETURN_FALSE(isInitialized() && !m_isFrameActive && _path);

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(resourceController);

    const BinaryResourcePointer fontResource = resourceController->load<BinaryResource>(_path);
    EGO_CHECK_RETURN_FALSE(fontResource && fontResource->isLoaded());

    return m_backend->setFont(fontResource->getContent(), _size);
}

bool ego::gui::GuiController::setStyle(const GuiStyle& _style)
{
    EGO_CHECK_RETURN_FALSE(isInitialized() && !m_isFrameActive);

    return m_backend->setStyle(_style);
}

void ego::gui::GuiController::update(float _deltaTime)
{
    EGO_CHECK_RETURN(isInitialized() && !m_isFrameActive);

    m_isFrameActive = true;

    GuiRenderData frame;
    const bool isFrameBuilt = m_backend->update(
        _deltaTime,
        [this]()
        {
            return drawLayers();
        },
        frame);
    if (isFrameBuilt)
    {
        m_pendingFrame = std::move(frame);
    }

    m_isFrameActive = false;
}

ego::gui::GuiRenderData ego::gui::GuiController::takeRenderData()
{
    EGO_ASSERT(!m_isFrameActive);

    GuiRenderData renderData = std::move(m_pendingFrame);
    m_pendingFrame = GuiRenderData();

    return renderData;
}

bool ego::gui::GuiController::registerLayer(GuiLayer& _layer)
{
    EGO_CHECK_RETURN_FALSE(isInitialized() && !m_isFrameActive);

    const LayerIterator existingLayer = findLayer(_layer);
    if (existingLayer != m_layers.end())
    {
        return true;
    }

    m_layers.push_back({.m_layer = _layer});

    return true;
}

bool ego::gui::GuiController::unregisterLayer(GuiLayer& _layer)
{
    EGO_CHECK_RETURN_FALSE(!m_isFrameActive);

    const LayerIterator layer = findLayer(_layer);
    EGO_CHECK_RETURN_FALSE(layer != m_layers.end());

    m_layers.erase(layer);

    return true;
}

bool ego::gui::GuiController::isInitialized() const
{
    return m_backend && m_backend->isInitialized();
}

bool ego::gui::GuiController::drawLayers()
{
    for (LayerRecord& layerRecord : m_layers)
    {
        if (!layerRecord.m_layer.get().draw(*m_backend))
        {
            return false;
        }
    }

    return true;
}

ego::gui::GuiController::LayerIterator ego::gui::GuiController::findLayer(GuiLayer& _layer)
{
    return std::ranges::find_if(
        m_layers,
        [&_layer](const LayerRecord& _record)
        {
            return std::addressof(_record.m_layer.get()) == std::addressof(_layer);
        });
}
