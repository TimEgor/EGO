#include "ImGuiRendererAdapter.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Implementation/ImGui/Platform/ImGuiPlatformAdapter.h"

#include <imgui.h>

namespace
{
    constexpr const char* BackendName = "EgoGuiRender";

    void DrawCallbackResetRenderState(const ImDrawList*, const ImDrawCmd*) {}

    void DrawCallbackSetSamplerLinear(const ImDrawList*, const ImDrawCmd*) {}

    void DrawCallbackSetSamplerNearest(const ImDrawList*, const ImDrawCmd*) {}
} // namespace

bool ego::gui::ImGuiRendererAdapter::init()
{
    EGO_CHECK_RETURN_FALSE(ImGui::GetCurrentContext() && !m_isInitialized);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    EGO_CHECK_RETURN_FALSE(!io.BackendRendererUserData);
    EGO_CHECK_RETURN_FALSE(
        !platformIO.Renderer_RenderState && !platformIO.DrawCallback_ResetRenderState && !platformIO.DrawCallback_SetSamplerLinear &&
        !platformIO.DrawCallback_SetSamplerNearest);

    io.BackendRendererName = BackendName;
    io.BackendRendererUserData = &m_userData;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    platformIO.DrawCallback_ResetRenderState = DrawCallbackResetRenderState;
    platformIO.DrawCallback_SetSamplerLinear = DrawCallbackSetSamplerLinear;
    platformIO.DrawCallback_SetSamplerNearest = DrawCallbackSetSamplerNearest;

    m_isInitialized = true;

    return true;
}

void ego::gui::ImGuiRendererAdapter::release()
{
    m_frameTextures.reset();
    m_userData.m_textureStorage.release();

    if (!ImGui::GetCurrentContext())
    {
        m_isInitialized = false;

        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    EGO_ASSERT(!io.BackendRendererUserData || io.BackendRendererUserData == &m_userData);
    if (io.BackendRendererUserData == &m_userData)
    {
        io.BackendRendererName = nullptr;
        io.BackendRendererUserData = nullptr;
        io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
        io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
        io.BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;

        ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
        platformIO.Renderer_RenderState = nullptr;
        platformIO.DrawCallback_ResetRenderState = nullptr;
        platformIO.DrawCallback_SetSamplerLinear = nullptr;
        platformIO.DrawCallback_SetSamplerNearest = nullptr;
    }

    m_isInitialized = false;
}

void ego::gui::ImGuiRendererAdapter::beginFrame()
{
    EGO_ASSERT(m_isInitialized);
    m_frameTextures.reset();
}

ego::gui::GuiFrameTextureID ego::gui::ImGuiRendererAdapter::bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode)
{
    EGO_CHECK_RETURN_VALUE(m_isInitialized, InvalidGuiFrameTextureID);

    return m_frameTextures.bind(_textureView, _samplingMode);
}

bool ego::gui::ImGuiRendererAdapter::buildRenderData(const ImGuiPlatformAdapter& _platform, GuiRenderData& _renderData)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);

    const ImGuiPlatformRenderTargetCollection renderTargets = _platform.getRenderTargets();
    _renderData.m_viewports.reserve(renderTargets.size());
    for (const ImGuiPlatformRenderTarget& renderTarget : renderTargets)
    {
        EGO_CHECK_RETURN_FALSE(renderTarget.m_drawData && renderTarget.m_graphicPresenter);
        EGO_CHECK_RETURN_FALSE(appendViewport(*renderTarget.m_drawData, renderTarget.m_graphicPresenter, _renderData));
    }

    return true;
}

bool ego::gui::ImGuiRendererAdapter::bindImGuiTextures(const ImDrawData& _drawData)
{
    if (!_drawData.Textures)
    {
        return true;
    }

    for (ImTextureData* textureData : *_drawData.Textures)
    {
        EGO_CHECK_RETURN_FALSE(textureData);
        EGO_CHECK_RETURN_FALSE(m_userData.m_textureStorage.synchronize(*textureData));
        if (textureData->Status == ImTextureStatus_Destroyed)
        {
            continue;
        }

        EGO_CHECK_RETURN_FALSE(textureData->Status == ImTextureStatus_OK);

        const gpu::TextureViewPointer textureView = m_userData.m_textureStorage.getTextureView(*textureData);
        EGO_CHECK_RETURN_FALSE(textureView);

        const GuiFrameTextureID textureID = m_frameTextures.bind(textureView, GetSamplingMode(*textureData));
        EGO_CHECK_RETURN_FALSE(textureID != InvalidGuiFrameTextureID);

        textureData->SetTexID(static_cast<ImTextureID>(textureID));
    }

    return true;
}

bool ego::gui::ImGuiRendererAdapter::appendViewport(const ImDrawData& _drawData, const GraphicPresenterPointer& _graphicPresenter, GuiRenderData& _renderData)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(bindImGuiTextures(_drawData));
    EGO_CHECK_RETURN_FALSE(_drawData.Valid);
    EGO_CHECK_RETURN_FALSE(_graphicPresenter);
    EGO_CHECK_RETURN_FALSE(_drawData.CmdListsCount >= 0 && _drawData.TotalVtxCount >= 0 && _drawData.TotalIdxCount >= 0);
    EGO_CHECK_RETURN_FALSE(static_cast<uint64_t>(_drawData.TotalVtxCount) <= (std::numeric_limits<uint32_t>::max)());
    EGO_CHECK_RETURN_FALSE(static_cast<uint64_t>(_drawData.TotalIdxCount) <= (std::numeric_limits<uint32_t>::max)());

    ViewportRenderData viewportRenderData;
    viewportRenderData.m_graphicPresenter = _graphicPresenter;

    DrawData& drawData = viewportRenderData.m_drawData;
    drawData.m_viewportSize = FloatVector2(_drawData.DisplaySize.x, _drawData.DisplaySize.y);
    drawData.m_framebufferScale = FloatVector2(_drawData.FramebufferScale.x, _drawData.FramebufferScale.y);
    drawData.m_vertices.reserve(static_cast<size_t>(_drawData.TotalVtxCount));
    drawData.m_indices.reserve(static_cast<size_t>(_drawData.TotalIdxCount));

    uint32_t globalVertexOffset = 0;
    uint32_t globalIndexOffset = 0;
    TextureFilteringMode filteringMode = TextureFilteringMode::Linear;
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();

    for (int listIndex = 0; listIndex < _drawData.CmdListsCount; ++listIndex)
    {
        const ImDrawList* sourceList = _drawData.CmdLists[listIndex];
        EGO_CHECK_RETURN_FALSE(sourceList);

        for (const ImDrawVert& sourceVertex : sourceList->VtxBuffer)
        {
            drawData.m_vertices.push_back(ConvertVertex(sourceVertex, _drawData.DisplayPos.x, _drawData.DisplayPos.y));
        }

        for (const ImDrawIdx sourceIndex : sourceList->IdxBuffer)
        {
            drawData.m_indices.push_back(static_cast<uint32_t>(sourceIndex));
        }

        for (const ImDrawCmd& sourceCommand : sourceList->CmdBuffer)
        {
            if (sourceCommand.UserCallback)
            {
                if (sourceCommand.UserCallback == platformIO.DrawCallback_ResetRenderState)
                {
                    filteringMode = TextureFilteringMode::Linear;
                    continue;
                }
                if (sourceCommand.UserCallback == platformIO.DrawCallback_SetSamplerLinear)
                {
                    filteringMode = TextureFilteringMode::Linear;
                    continue;
                }
                if (sourceCommand.UserCallback == platformIO.DrawCallback_SetSamplerNearest)
                {
                    filteringMode = TextureFilteringMode::Nearest;
                    continue;
                }

                EGO_ASSERT_FAIL_MESSAGE(
                    "EgoGui render packets support only reset-render-state and linear/nearest sampler callbacks. "
                    "Arbitrary ImDrawCallback functions cannot cross the backend-neutral rendering boundary.");

                return false;
            }

            EGO_CHECK_RETURN_FALSE(appendCommand(
                sourceCommand,
                globalIndexOffset,
                globalVertexOffset,
                _drawData.DisplayPos.x,
                _drawData.DisplayPos.y,
                filteringMode,
                _renderData,
                drawData));
        }

        EGO_CHECK_RETURN_FALSE(
            static_cast<uint64_t>(globalVertexOffset) + static_cast<uint64_t>(sourceList->VtxBuffer.Size) <= (std::numeric_limits<uint32_t>::max)());
        EGO_CHECK_RETURN_FALSE(
            static_cast<uint64_t>(globalIndexOffset) + static_cast<uint64_t>(sourceList->IdxBuffer.Size) <= (std::numeric_limits<uint32_t>::max)());

        globalVertexOffset += static_cast<uint32_t>(sourceList->VtxBuffer.Size);
        globalIndexOffset += static_cast<uint32_t>(sourceList->IdxBuffer.Size);
    }

    _renderData.m_viewports.push_back(std::move(viewportRenderData));

    return true;
}

ego::gui::Vertex ego::gui::ImGuiRendererAdapter::ConvertVertex(const ImDrawVert& _vertex, float _displayPositionX, float _displayPositionY)
{
    constexpr float inverseColorScale = 1.0f / 255.0f;

    Vertex vertex;
    vertex.m_position = FloatVector2(_vertex.pos.x - _displayPositionX, _vertex.pos.y - _displayPositionY);
    vertex.m_uv = FloatVector2(_vertex.uv.x, _vertex.uv.y);
    vertex.m_color = FloatVector4(
        static_cast<float>((_vertex.col >> IM_COL32_R_SHIFT) & 0xFFu) * inverseColorScale,
        static_cast<float>((_vertex.col >> IM_COL32_G_SHIFT) & 0xFFu) * inverseColorScale,
        static_cast<float>((_vertex.col >> IM_COL32_B_SHIFT) & 0xFFu) * inverseColorScale,
        static_cast<float>((_vertex.col >> IM_COL32_A_SHIFT) & 0xFFu) * inverseColorScale);

    return vertex;
}

ego::gui::TextureSamplingMode ego::gui::ImGuiRendererAdapter::GetSamplingMode(const ImTextureData& _textureData)
{
    return _textureData.Format == ImTextureFormat_Alpha8 ? TextureSamplingMode::Alpha : TextureSamplingMode::Color;
}

bool ego::gui::ImGuiRendererAdapter::appendCommand(
    const ImDrawCmd& _source,
    uint32_t _globalIndexOffset,
    uint32_t _globalVertexOffset,
    float _displayPositionX,
    float _displayPositionY,
    TextureFilteringMode _filteringMode,
    GuiRenderData& _renderData,
    DrawData& _drawData) const
{
    EGO_CHECK_RETURN_FALSE(static_cast<uint64_t>(_globalIndexOffset) + static_cast<uint64_t>(_source.IdxOffset) <= (std::numeric_limits<uint32_t>::max)());
    EGO_CHECK_RETURN_FALSE(
        static_cast<uint64_t>(_globalVertexOffset) + static_cast<uint64_t>(_source.VtxOffset) <= static_cast<uint64_t>((std::numeric_limits<int32_t>::max)()));

    DrawCommand command;
    command.m_clipRect = FloatVector4(
        _source.ClipRect.x - _displayPositionX,
        _source.ClipRect.y - _displayPositionY,
        _source.ClipRect.z - _displayPositionX,
        _source.ClipRect.w - _displayPositionY);
    command.m_firstIndex = _globalIndexOffset + _source.IdxOffset;
    command.m_indexCount = _source.ElemCount;
    command.m_vertexOffset = static_cast<int32_t>(_globalVertexOffset + _source.VtxOffset);
    command.m_textureFilteringMode = _filteringMode;

    const GuiFrameTextureID textureID = static_cast<GuiFrameTextureID>(_source.GetTexID());
    if (textureID != InvalidGuiFrameTextureID)
    {
        ImGuiFrameTextures::Texture texture;
        EGO_CHECK_RETURN_FALSE(m_frameTextures.resolve(textureID, texture));
        EGO_CHECK_RETURN_FALSE(texture.m_textureView);
        EGO_CHECK_RETURN_FALSE(texture.m_textureView->getBindlessIndex() != gpu::InvalidBindlessIndex);

        command.m_textureIndex = texture.m_textureView->getBindlessIndex();
        command.m_textureSamplingMode = texture.m_samplingMode;

        const GuiRenderData::TextureViewCollection::const_iterator resourceIt = std::find_if(
            _renderData.m_resourceTextureViews.cbegin(),
            _renderData.m_resourceTextureViews.cend(),
            [&texture](const gpu::TextureViewPointer& _resourceTextureView)
            {
                return _resourceTextureView.getObject() == texture.m_textureView.getObject();
            });
        if (resourceIt == _renderData.m_resourceTextureViews.cend())
        {
            _renderData.m_resourceTextureViews.push_back(texture.m_textureView);
        }
    }

    _drawData.m_commands.push_back(command);

    return true;
}
