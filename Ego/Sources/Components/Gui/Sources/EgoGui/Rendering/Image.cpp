#include "Image.h"

#include <cstddef>
#include <utility>

ego::gui::ImagePointer ego::gui::Image::Create(ImageID _id, uint32_t _width, uint32_t _height, ImageFormat _format, PixelCollection _pixels)
{
    const size_t expectedPixelCount = static_cast<size_t>(_width) * static_cast<size_t>(_height);
    if (_id == InvalidImageID || _width == 0 || _height == 0 || _format != ImageFormat::R8 || _pixels.size() != expectedPixelCount)
    {
        return nullptr;
    }

    return new Image(_id, _width, _height, _format, std::move(_pixels));
}

ego::gui::Image::Image(ImageID _id, uint32_t _width, uint32_t _height, ImageFormat _format, PixelCollection&& _pixels)
    : m_id(_id),
      m_width(_width),
      m_height(_height),
      m_format(_format),
      m_pixels(std::move(_pixels))
{
}

ego::gui::ImageID ego::gui::Image::getID() const
{
    return m_id;
}

uint32_t ego::gui::Image::getWidth() const
{
    return m_width;
}

uint32_t ego::gui::Image::getHeight() const
{
    return m_height;
}

ego::gui::ImageFormat ego::gui::Image::getFormat() const
{
    return m_format;
}

const ego::gui::Image::PixelCollection& ego::gui::Image::getPixels() const
{
    return m_pixels;
}
