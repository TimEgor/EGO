#include "ImGuiBackend.h"

#include <cmath>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Implementation/ImGui/ImGuiContextScope.h"

#include <imgui.h>

ego::gui::ImGuiBackend::ImGuiBackend() = default;

ego::gui::ImGuiBackend::~ImGuiBackend()
{
    release();
}

bool ego::gui::ImGuiBackend::init(const ViewportProviderPointer& _viewportProvider, bool _enableMultiViewport)
{
    EGO_CHECK_RETURN_FALSE(!m_isInitialized && !m_context);
    EGO_CHECK_RETURN_FALSE(_viewportProvider);

    EGO_CHECK_INITIALIZATION(initializeContext(_enableMultiViewport));

    bool adaptersInitialized = false;
    {
        const ImGuiContextScope contextScope(m_context);
        adaptersInitialized = contextScope.isActive() && m_rendererAdapter.init() && m_platformAdapter.init(_viewportProvider);
    }
    EGO_CHECK_INITIALIZATION(adaptersInitialized);

    m_isInitialized = true;

    return true;
}

bool ego::gui::ImGuiBackend::release()
{
    EGO_ASSERT(!m_isFrameActive);
    if (m_isFrameActive || !releaseContext())
    {
        return false;
    }

    resetState();

    return true;
}

bool ego::gui::ImGuiBackend::beginFrame(float _deltaTime)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized && !m_isFrameActive);
    EGO_CHECK_RETURN_FALSE(InitializeImGuiModuleRuntime());

    m_previousContext = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(m_context);

    if (!m_platformAdapter.beginFrame())
    {
        restorePreviousContext();

        return false;
    }

    static constexpr float DefaultDeltaTime = 1.0f / 60.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = std::isfinite(_deltaTime) && _deltaTime > 0.0f ? _deltaTime : DefaultDeltaTime;

    m_rendererAdapter.beginFrame();
    ImGui::NewFrame();
    m_isFrameActive = true;

    return true;
}

bool ego::gui::ImGuiBackend::endFrame(GuiRenderData& _renderData)
{
    EGO_CHECK_RETURN_FALSE(m_isFrameActive);

    EGO_ASSERT(ImGui::GetCurrentContext() == m_context);
    if (ImGui::GetCurrentContext() != m_context)
    {
        m_isFrameActive = false;
        restorePreviousContext();

        return false;
    }

    ImGui::Render();

    GuiRenderData renderData;
    const bool frameBuilt = m_platformAdapter.endFrame() && m_rendererAdapter.buildRenderData(m_platformAdapter, renderData);
    if (frameBuilt)
    {
        _renderData = std::move(renderData);
    }

    m_isFrameActive = false;
    restorePreviousContext();

    return frameBuilt;
}

void ego::gui::ImGuiBackend::cancelFrame()
{
    EGO_CHECK_RETURN(m_isFrameActive);

    EGO_ASSERT(ImGui::GetCurrentContext() == m_context);
    if (ImGui::GetCurrentContext() == m_context)
    {
        ImGui::EndFrame();
    }

    m_isFrameActive = false;
    restorePreviousContext();
}

ego::gui::GuiFrameTextureID ego::gui::ImGuiBackend::bindTexture(const gpu::TextureViewReference& _textureView, TextureSamplingMode _samplingMode)
{
    EGO_CHECK_RETURN_VALUE(m_isFrameActive, InvalidGuiFrameTextureID);

    return m_rendererAdapter.bindTexture(_textureView, _samplingMode);
}

bool ego::gui::ImGuiBackend::isInitialized() const
{
    return m_isInitialized;
}

bool ego::gui::ImGuiBackend::initializeContext(bool _enableMultiViewport)
{
    EGO_CHECK_RETURN_FALSE(InitializeImGuiModuleRuntime());

    ImGuiContext* previousContext = ImGui::GetCurrentContext();
    m_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(previousContext);
    EGO_CHECK_RETURN_FALSE(m_context);

    const ImGuiContextScope contextScope(m_context);
    EGO_CHECK_RETURN_FALSE(contextScope.isActive());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (_enableMultiViewport)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    EGO_CHECK_RETURN_FALSE(io.Fonts->AddFontDefault());

    return true;
}

bool ego::gui::ImGuiBackend::releaseContext()
{
    if (!m_context)
    {
        return true;
    }

    {
        const ImGuiContextScope contextScope(m_context);
        EGO_ASSERT_MESSAGE(contextScope.isActive(), "GUI backend must be released on its runtime thread.");
        if (!contextScope.isActive())
        {
            return false;
        }

        m_platformAdapter.release();
        m_rendererAdapter.release();
    }

    ImGui::DestroyContext(m_context);
    m_context = nullptr;

    return true;
}

void ego::gui::ImGuiBackend::resetState()
{
    EGO_ASSERT(!m_context && !m_previousContext && !m_isFrameActive);

    m_isInitialized = false;
}

void ego::gui::ImGuiBackend::restorePreviousContext()
{
    ImGui::SetCurrentContext(m_previousContext);
    m_previousContext = nullptr;
}
