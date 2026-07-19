#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ego::gui
{
    bool DecodeNextUtf8(std::string_view _text, size_t& _byteOffset, uint32_t& _codepoint);
    bool AppendUtf8Codepoint(std::string& _text, uint32_t _codepoint);
} // namespace ego::gui
