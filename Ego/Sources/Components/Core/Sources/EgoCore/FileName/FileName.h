#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ego
{
    using FileNameID = uint32_t;
    inline constexpr FileNameID InvalidFileNameID = 0;

    class FileName final
    {
    public:
        using CharType = char;
        using StringType = std::basic_string<CharType>;
        using StringView = std::basic_string_view<CharType>;

        FileName() = default;
        FileName(StringView _name);
        FileName(CharType _character);
        FileName(const FileName&) = default;
        FileName(FileName&&) noexcept = default;
        FileName(std::nullptr_t) = delete;

        FileName& operator=(const FileName&) = default;
        FileName& operator=(FileName&&) noexcept = default;
        FileName& operator=(std::nullptr_t) = delete;
        FileName& operator=(StringView _name);
        FileName& operator=(CharType _character);

        FileName& operator+=(StringView _name);
        FileName& operator+=(CharType _character);
        FileName& operator+=(const FileName& _name);
        FileName& operator+=(std::nullptr_t) = delete;

        bool operator==(const FileName& _name) const;
        bool operator==(std::nullptr_t) const = delete;

        explicit operator bool() const;

        const CharType& operator[](size_t _index) const;
        CharType& operator[](size_t _index);

        StringView getView() const;
        const CharType* c_str() const;
        size_t length() const;
        bool empty() const;

        void clear();
        void release();

        FileNameID hash() const;

    private:
        static bool IsValid(StringView _name);

        void assign(StringView _name);
        void append(StringView _name);
        void normalize(size_t _beginIndex);

        StringType m_name;
    };

    FileName operator+(FileName _name, FileName::StringView _suffix);
    FileName operator+(FileName::StringView _prefix, const FileName& _name);
    FileName operator+(FileName _name, FileName::CharType _character);
    FileName operator+(FileName::CharType _character, const FileName& _name);
    FileName operator+(FileName _left, const FileName& _right);
} // namespace ego
