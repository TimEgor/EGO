#include "TextureStorage.h"

#include <cstdint>
#include <limits>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include <imgui.h>

ego::gui::TextureStorage::~TextureStorage()
{
    release();
}

bool ego::gui::TextureStorage::synchronize(ImTextureData& _textureData)
{
    switch (_textureData.Status)
    {
    case ImTextureStatus_OK:
        return m_textures.contains(&_textureData);
    case ImTextureStatus_Destroyed:
        return true;
    case ImTextureStatus_WantCreate:
        return createTexture(_textureData);
    case ImTextureStatus_WantUpdates:
        return replaceTexture(_textureData);
    case ImTextureStatus_WantDestroy:
        destroyTexture(_textureData);

        return true;
    }

    return false;
}

ego::gpu::TextureViewPointer ego::gui::TextureStorage::getTextureView(ImTextureData& _textureData) const
{
    const TextureCollection::const_iterator textureIt = m_textures.find(&_textureData);
    if (textureIt == m_textures.end())
    {
        return nullptr;
    }

    return textureIt->second;
}

void ego::gui::TextureStorage::release()
{
    for (TextureCollection::value_type& textureEntry : m_textures)
    {
        ImTextureData* textureData = textureEntry.first;
        if (textureData)
        {
            textureData->SetTexID(ImTextureID_Invalid);
            textureData->BackendUserData = nullptr;
            textureData->SetStatus(ImTextureStatus_Destroyed);
        }
    }

    m_textures.clear();
}

bool ego::gui::TextureStorage::createTexture(ImTextureData& _textureData)
{
    if (m_textures.contains(&_textureData))
    {
        destroyTexture(_textureData);
    }

    const gpu::TextureViewPointer textureView = createTextureResource(_textureData);
    EGO_CHECK_RETURN_FALSE(textureView);

    const std::pair<TextureCollection::iterator, bool> insertResult = m_textures.emplace(&_textureData, textureView);
    if (!insertResult.second)
    {
        return false;
    }

    _textureData.BackendUserData = nullptr;
    _textureData.SetStatus(ImTextureStatus_OK);

    return true;
}

bool ego::gui::TextureStorage::replaceTexture(ImTextureData& _textureData)
{
    TextureCollection::iterator textureIt = m_textures.find(&_textureData);
    if (textureIt == m_textures.end())
    {
        return createTexture(_textureData);
    }

    const gpu::TextureViewPointer textureView = createTextureResource(_textureData);
    EGO_CHECK_RETURN_FALSE(textureView);
    textureIt->second = textureView;

    _textureData.SetStatus(ImTextureStatus_OK);

    return true;
}

void ego::gui::TextureStorage::destroyTexture(ImTextureData& _textureData)
{
    const TextureCollection::iterator textureIt = m_textures.find(&_textureData);
    if (textureIt != m_textures.end())
    {
        m_textures.erase(textureIt);
    }

    _textureData.SetTexID(ImTextureID_Invalid);
    _textureData.BackendUserData = nullptr;
    _textureData.SetStatus(ImTextureStatus_Destroyed);
}

ego::gpu::TextureViewPointer ego::gui::TextureStorage::createTextureResource(ImTextureData& _textureData) const
{
    EGO_CHECK_RETURN_NULL(_textureData.Width > 0 && _textureData.Height > 0);
    EGO_CHECK_RETURN_NULL(_textureData.Pixels);
    EGO_CHECK_RETURN_NULL(_textureData.GetSizeInBytes() > 0);
    EGO_CHECK_RETURN_NULL(static_cast<uint64_t>(_textureData.GetSizeInBytes()) <= (std::numeric_limits<uint32_t>::max)());

    gpu::GraphicResourceFormat format = gpu::GraphicResourceFormat::Undefined;
    switch (_textureData.Format)
    {
    case ImTextureFormat_RGBA32:
        format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
        break;
    case ImTextureFormat_Alpha8:
        format = gpu::GraphicResourceFormat::R8UNorm;
        break;
    }
    EGO_CHECK_RETURN_NULL(format != gpu::GraphicResourceFormat::Undefined);

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_size = gpu::Texture2DSize(static_cast<uint32_t>(_textureData.Width), static_cast<uint32_t>(_textureData.Height));
    textureDesc.m_format = format;
    textureDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GraphicResourceUsageTransferDst | gpu::GraphicResourceUsageShaderResource);
    textureDesc.m_access = gpu::GraphicResourceAccessGpuRead;

    const gpu::InitialGraphicResourceData initialData(_textureData.Pixels, static_cast<uint32_t>(_textureData.GetSizeInBytes()));
    gpu::GpuOperationOptions operationOptions;
    operationOptions.m_completionMode = gpu::GpuCompletionMode::WaitForCompletion;

    const gpu::GpuTexture2DTicket textureTicket = gpu::GetGraphicDevice().createAndUploadTexture2D(textureDesc, initialData, operationOptions);
    EGO_CHECK_RETURN_NULL(textureTicket);
    textureTicket.waitReady();
    EGO_CHECK_RETURN_NULL(textureTicket.isReady());

    gpu::TextureViewDesc textureViewDesc;
    textureViewDesc.m_type = gpu::GraphicResourceViewType::ShaderResource;
    textureViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    textureViewDesc.m_format = format;

    const gpu::TextureViewPointer textureView = gpu::GetGraphicDevice().createTextureView(textureTicket.m_resource, textureViewDesc);
    EGO_CHECK_RETURN_NULL(textureView);
    EGO_CHECK_RETURN_NULL(textureView->getBindlessIndex() != gpu::InvalidBindlessIndex);

    return textureView;
}
