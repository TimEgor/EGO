#pragma once

#include "EgoMath/Vector.h"

#include "Format.h"
#include "GraphicResource.h"
#include "ResourceView.h"

namespace ego::gpu
{
    enum TextureUsage
    {
        TextureUsageRenderTarget = GraphicResourceMaxUsage << 1,
        TextureUsageDepthStencil = GraphicResourceMaxUsage << 2
    };

    enum class TextureDimension
    {
        D1,
        D2,
        D3
    };

    struct TextureSampleDesc final
    {
        uint32_t m_count = 1;
        uint32_t m_quality = 0;
    };

    class Texture : public GraphicResource
    {
    public:
        Texture() = default;

        virtual TextureDimension getDimension() const = 0;

        EGO_GRAPHIC_RESOURCE(Texture, GraphicResource);
    };

    EGO_REFERENCE(Texture);

    using Texture2DSize = UInt32Vector2;

    struct Texture2DDesc final : GraphicResourceDesc
    {
        Texture2DSize m_size = UInt32Vector2Zero;
        uint32_t m_arrayLayers = 1;
        uint32_t m_mipLevels = 1;
        TextureSampleDesc m_samples;
        GraphicResourceFormat m_format = GraphicResourceFormat::Undefined;
    };

    class Texture2D : public Texture
    {
    public:
        Texture2D(const Texture2DDesc& _desc);

        const Texture2DDesc& getDesc() const;

        TextureDimension getDimension() const override;

        EGO_GRAPHIC_RESOURCE(Texture2D, Texture);

    private:
        Texture2DDesc m_desc;
    };

    EGO_REFERENCE(Texture2D);

    enum class TextureViewDimension
    {
        D1,
        D2,
        D3,
    };

    struct TextureViewDesc final
    {
        GraphicResourceViewType m_type = GraphicResourceViewType::ShaderResource;
        TextureViewDimension m_dimension = TextureViewDimension::D2;
        GraphicResourceFormat m_format = GraphicResourceFormat::Undefined;
    };

    class TextureView : public ResourceView
    {
    public:
        TextureView(const TextureReference& _texture, const TextureViewDesc& _desc);

        const TextureViewDesc& getDesc() const;
        GraphicResourceViewType getViewType() const override;

    private:
        TextureViewDesc m_desc;
    };

    EGO_REFERENCE(TextureView);
} // namespace ego::gpu
