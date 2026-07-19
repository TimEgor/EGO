#include "TextEncoding.h"

namespace
{
    constexpr uint32_t ReplacementCodepoint = 0xFFFD;

    bool IsContinuationByte(unsigned char _byte)
    {
        return (_byte & 0xC0) == 0x80;
    }
} // namespace

bool ego::gui::DecodeNextUtf8(std::string_view _text, size_t& _byteOffset, uint32_t& _codepoint)
{
    if (_byteOffset >= _text.size())
    {
        return false;
    }

    const unsigned char firstByte = static_cast<unsigned char>(_text[_byteOffset]);
    if (firstByte < 0x80)
    {
        _codepoint = firstByte;
        ++_byteOffset;
        return true;
    }

    uint32_t codepoint = 0;
    size_t sequenceLength = 0;
    if ((firstByte & 0xE0) == 0xC0)
    {
        codepoint = firstByte & 0x1F;
        sequenceLength = 2;
    }
    else if ((firstByte & 0xF0) == 0xE0)
    {
        codepoint = firstByte & 0x0F;
        sequenceLength = 3;
    }
    else if ((firstByte & 0xF8) == 0xF0)
    {
        codepoint = firstByte & 0x07;
        sequenceLength = 4;
    }
    else
    {
        _codepoint = ReplacementCodepoint;
        ++_byteOffset;
        return true;
    }

    if (_byteOffset + sequenceLength > _text.size())
    {
        _codepoint = ReplacementCodepoint;
        ++_byteOffset;
        return true;
    }

    for (size_t byteIndex = 1; byteIndex < sequenceLength; ++byteIndex)
    {
        const unsigned char continuationByte = static_cast<unsigned char>(_text[_byteOffset + byteIndex]);
        if (!IsContinuationByte(continuationByte))
        {
            _codepoint = ReplacementCodepoint;
            ++_byteOffset;
            return true;
        }

        codepoint = (codepoint << 6) | (continuationByte & 0x3F);
    }

    const bool isOverlong = (sequenceLength == 2 && codepoint < 0x80) || (sequenceLength == 3 && codepoint < 0x800) || (sequenceLength == 4 && codepoint < 0x10000);
    const bool isInvalidCodepoint = codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF);
    _codepoint = isOverlong || isInvalidCodepoint ? ReplacementCodepoint : codepoint;
    _byteOffset += sequenceLength;
    return true;
}

bool ego::gui::AppendUtf8Codepoint(std::string& _text, uint32_t _codepoint)
{
    if (_codepoint > 0x10FFFF || (_codepoint >= 0xD800 && _codepoint <= 0xDFFF))
    {
        return false;
    }

    if (_codepoint <= 0x7F)
    {
        _text.push_back(static_cast<char>(_codepoint));
    }
    else if (_codepoint <= 0x7FF)
    {
        _text.push_back(static_cast<char>(0xC0 | (_codepoint >> 6)));
        _text.push_back(static_cast<char>(0x80 | (_codepoint & 0x3F)));
    }
    else if (_codepoint <= 0xFFFF)
    {
        _text.push_back(static_cast<char>(0xE0 | (_codepoint >> 12)));
        _text.push_back(static_cast<char>(0x80 | ((_codepoint >> 6) & 0x3F)));
        _text.push_back(static_cast<char>(0x80 | (_codepoint & 0x3F)));
    }
    else
    {
        _text.push_back(static_cast<char>(0xF0 | (_codepoint >> 18)));
        _text.push_back(static_cast<char>(0x80 | ((_codepoint >> 12) & 0x3F)));
        _text.push_back(static_cast<char>(0x80 | ((_codepoint >> 6) & 0x3F)));
        _text.push_back(static_cast<char>(0x80 | (_codepoint & 0x3F)));
    }

    return true;
}
