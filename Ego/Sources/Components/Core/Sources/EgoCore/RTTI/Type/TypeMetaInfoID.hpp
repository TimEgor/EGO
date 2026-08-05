#include <type_traits>

#include "EgoCore/Hash/CRC32.h"

namespace ego::rtti
{
    template <typename T>
    constexpr TypeMetaInfoID GetTypeMetaInfoID()
    {
        using Type = std::remove_cvref_t<T>;

        if constexpr (requires { Type::GetMetaInfoID(); })
        {
            return Type::GetMetaInfoID();
        }
        else
        {
            return GetTypeMetaInfoID(__FUNCSIG__);
        }
    }

    constexpr TypeMetaInfoID GetTypeMetaInfoID(const char* _typeName)
    {
        return _typeName ? Crc32(_typeName) : InvalidTypeMetaInfoID;
    }
} // namespace ego::rtti
