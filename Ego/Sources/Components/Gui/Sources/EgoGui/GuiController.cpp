#include "EgoGui/GuiController.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Viewport/ViewportManager.h"

ego::gui::GuiController::VisualOperationScope::VisualOperationScope(GuiController& _controller)
    : m_controller(_controller)
{
    EGO_ASSERT(!m_controller.m_isVisualOperationActive);
    m_controller.m_isVisualOperationActive = true;
}

ego::gui::GuiController::VisualOperationScope::~VisualOperationScope()
{
    EGO_ASSERT(m_controller.m_isVisualOperationActive);
    m_controller.m_isVisualOperationActive = false;
}

ego::gui::GuiController::GuiController()
    : m_viewportManager(std::make_unique<ViewportManager>()),
      m_theme(SharedPointer<Theme>(new Theme(Theme::GetDefault())))
{
}

ego::gui::GuiController::~GuiController()
{
    release();
}

bool ego::gui::GuiController::init(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(ensureVisualOperationInactive());

    const InitData initData = _initData;
    release();

    EGO_CHECK_RETURN_FALSE(initData.m_viewportProvider);

    setTheme(initData.m_theme);

    if (!initData.m_fontAtlasDesc.m_fontData.empty())
    {
        m_fontAtlas = FontAtlasPointer(new FontAtlas());
        EGO_CHECK_RETURN_CALL_FALSE(m_fontAtlas && m_fontAtlas->init(initData.m_fontAtlasDesc), release());
    }

    EGO_CHECK_RETURN_CALL_FALSE(m_viewportManager->init(initData.m_viewportProvider, initData.m_enableMultiViewport), release());

    m_isInitialized = true;
    return true;
}

void ego::gui::GuiController::release()
{
    if (!ensureVisualOperationInactive())
    {
        return;
    }

    releaseState();
}

void ego::gui::GuiController::releaseState()
{
    m_viewportManager->release();
    m_fontAtlas = nullptr;
    m_isInitialized = false;
}

ego::gui::ViewportPointer ego::gui::GuiController::createViewport(const ViewportDesc& _desc)
{
    if (!m_isInitialized || !ensureVisualOperationInactive())
    {
        return nullptr;
    }

    return m_viewportManager->createViewport(_desc);
}

bool ego::gui::GuiController::destroyViewport(const ViewportPointer& _viewport)
{
    return m_isInitialized && ensureVisualOperationInactive() && m_viewportManager->destroyViewport(_viewport);
}

ego::gui::ViewportPointer ego::gui::GuiController::getPrimaryViewport() const
{
    return m_viewportManager->getPrimaryViewport();
}

ego::gui::ViewportPointer ego::gui::GuiController::findViewport(const WindowPointer& _window) const
{
    return m_viewportManager->findViewport(_window);
}

void ego::gui::GuiController::setMultiViewportEnabled(bool _isEnabled)
{
    if (!m_isInitialized || !ensureVisualOperationInactive())
    {
        return;
    }

    m_viewportManager->setMultiViewportEnabled(_isEnabled);
}

bool ego::gui::GuiController::isMultiViewportEnabled() const
{
    return m_viewportManager->isMultiViewportEnabled();
}

void ego::gui::GuiController::setTheme(const Theme& _theme)
{
    applyTheme(SharedPointer<Theme>(new Theme(_theme)));
}

ego::gui::ThemePointer ego::gui::GuiController::getTheme() const
{
    return m_theme;
}

void ego::gui::GuiController::update()
{
    if (!m_isInitialized || !ensureVisualOperationInactive())
    {
        return;
    }

    const ThemePointer theme = m_theme;
    const VisualOperationScope visualOperation(*this);
    const LayoutContext layoutContext{m_fontAtlas, theme};

    if (!m_viewportManager->update(layoutContext))
    {
        releaseState();
    }
}

ego::gui::GuiRenderData ego::gui::GuiController::buildFrame()
{
    if (!m_isInitialized || !ensureVisualOperationInactive())
    {
        return GuiRenderData();
    }

    const VisualOperationScope visualOperation(*this);
    const LayoutContext layoutContext{m_fontAtlas, m_theme};

    return m_viewportManager->buildFrame(layoutContext);
}

bool ego::gui::GuiController::isInitialized() const
{
    return m_isInitialized;
}

bool ego::gui::GuiController::ensureVisualOperationInactive() const
{
    const bool isInactive = !m_isVisualOperationActive;
    EGO_ASSERT_MESSAGE(isInactive, "Public visual operations cannot reenter an active update or frame build.");

    return isInactive;
}

void ego::gui::GuiController::applyTheme(ThemePointer _theme)
{
    m_theme = std::move(_theme);
    m_viewportManager->invalidateLayouts();
}
