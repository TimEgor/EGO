#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Reference/Pointer.h"

namespace ego::gui
{
    using ImageID = uint64_t;
    inline constexpr ImageID InvalidImageID = 0;

    enum class ImageFormat
    {
        R8
    };

    class Image;
    EGO_POINTER(Image);

    class Image final
    {
    public:
        using PixelCollection = std::vector<uint8_t>;

        static ImagePointer Create(ImageID _id, uint32_t _width, uint32_t _height, ImageFormat _format, PixelCollection _pixels);

        ImageID getID() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        ImageFormat getFormat() const;
        const PixelCollection& getPixels() const;

    private:
        Image(ImageID _id, uint32_t _width, uint32_t _height, ImageFormat _format, PixelCollection&& _pixels);

        const ImageID m_id;
        const uint32_t m_width;
        const uint32_t m_height;
        const ImageFormat m_format;
        const PixelCollection m_pixels;
    };

    struct ImageBinding final
    {
        ImageID m_id = InvalidImageID;
        ImagePointer m_image = nullptr;
    };

    using ImageBindingCollection = std::vector<ImageBinding>;
} // namespace ego::gui
