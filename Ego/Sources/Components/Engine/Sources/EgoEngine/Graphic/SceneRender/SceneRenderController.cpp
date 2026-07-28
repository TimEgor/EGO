#include "SceneRenderController.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

ego::engine::SceneRenderController::~SceneRenderController()
{
    release();
}

bool ego::engine::SceneRenderController::init(const render::RenderPluginPointer& _renderPlugin)
{
    EGO_CHECK_RETURN_FALSE(_renderPlugin);
    EGO_CHECK_RETURN_FALSE(!m_renderPlugin && !m_render);

    render::RenderPointer sceneRender = _renderPlugin->createRender();
    EGO_CHECK_RETURN_FALSE(sceneRender);

    if (!sceneRender->init())
    {
        sceneRender->release();
        return false;
    }

    m_renderPlugin = _renderPlugin;
    m_render = sceneRender;
    return true;
}

void ego::engine::SceneRenderController::release()
{
    if (m_render)
    {
        m_render->release();
        m_render = nullptr;
    }

    m_renderPlugin = nullptr;
    m_isFramePrepared = false;
}

void ego::engine::SceneRenderController::clearResources()
{
    if (m_render)
    {
        m_render->clearResources();
    }

    m_isFramePrepared = false;
}

bool ego::engine::SceneRenderController::prepare(const render::RenderPrepareContext& _context)
{
    m_isFramePrepared = false;

    if (!m_render)
    {
        EGO_ASSERT_FAIL_MESSAGE("SceneRenderController isn't initialized.");
        return false;
    }

    m_isFramePrepared = m_render->prepare(_context);
    return m_isFramePrepared;
}

void ego::engine::SceneRenderController::render(const gpu::TextureViewPointer& _targetView)
{
    if (!m_render || !m_isFramePrepared || !_targetView)
    {
        m_isFramePrepared = false;
        return;
    }

    m_render->render(_targetView);
    m_isFramePrepared = false;
}

void ego::engine::SceneRenderController::wait()
{
    if (m_render)
    {
        m_render->wait();
    }
}

ego::render::Render& ego::engine::SceneRenderController::getRender()
{
    EGO_ASSERT(m_render);
    return *m_render;
}

const ego::render::Render& ego::engine::SceneRenderController::getRender() const
{
    EGO_ASSERT(m_render);
    return *m_render;
}

bool ego::engine::SceneRenderController::isInitialized() const
{
    return m_renderPlugin && m_render;
}
