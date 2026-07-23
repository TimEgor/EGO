#pragma once

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class Image;
    EGO_POINTER(Image);

    class Image final : public Widget
    {
    public:
        static ImagePointer Create();
        static ImagePointer Create(const gpu::TextureViewReference& _textureView);

        void setTextureView(const gpu::TextureViewReference& _textureView);
        const gpu::TextureViewReference& getTextureView() const;

        EGO_RTTI_VIRTUAL(Image, Widget);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        Image() = default;

        gpu::TextureViewReference m_textureView = nullptr;
    };
} // namespace ego::gui
