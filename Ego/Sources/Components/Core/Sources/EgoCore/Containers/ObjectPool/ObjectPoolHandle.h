#pragma once

#include <cstdint>

namespace ego
{
    struct ObjectPoolHandleStorage8 final
    {
        using KeyType = uint8_t;
        using IndexType = uint8_t;
        using VersionType = uint8_t;

        static constexpr KeyType InvalidKey = 0;
        static constexpr VersionType InvalidVersion = 0;

        union
        {
            KeyType m_key = InvalidKey;

            struct
            {
                IndexType m_index : 4;
                VersionType m_version : 4;
            };
        };

        constexpr ObjectPoolHandleStorage8() = default;
        constexpr ObjectPoolHandleStorage8(KeyType _key): m_key(_key) {}

        constexpr ObjectPoolHandleStorage8(IndexType _index, VersionType _version)
            : m_index(_index),
              m_version(_version)
        {}

        constexpr ObjectPoolHandleStorage8(const ObjectPoolHandleStorage8&) = default;

        IndexType getIndex() const { return m_index; }
        VersionType getVersion() const { return m_version; }
    };

    struct ObjectPoolHandleStorage16 final
    {
        using KeyType = uint16_t;
        using IndexType = uint8_t;
        using VersionType = uint8_t;

        static constexpr KeyType InvalidKey = 0;
        static constexpr VersionType InvalidVersion = 0;

        union
        {
            KeyType m_key = InvalidKey;

            struct
            {
                IndexType m_index;
                VersionType m_version;
            };
        };

        constexpr ObjectPoolHandleStorage16() = default;
        constexpr ObjectPoolHandleStorage16(KeyType _key): m_key(_key) {}

        constexpr ObjectPoolHandleStorage16(IndexType _index, VersionType _version)
            : m_index(_index),
              m_version(_version)
        {}

        constexpr ObjectPoolHandleStorage16(const ObjectPoolHandleStorage16&) = default;

        IndexType getIndex() const { return m_index; }
        VersionType getVersion() const { return m_version; }
    };

    struct ObjectPoolHandleStorage32 final
    {
        using KeyType = uint32_t;
        using IndexType = uint16_t;
        using VersionType = uint16_t;

        static constexpr KeyType InvalidKey = 0;
        static constexpr VersionType InvalidVersion = 0;

        union
        {
            KeyType m_key = InvalidKey;

            struct
            {
                IndexType m_index;
                VersionType m_version;
            };
        };

        constexpr ObjectPoolHandleStorage32() = default;
        constexpr ObjectPoolHandleStorage32(KeyType _key): m_key(_key) {}

        constexpr ObjectPoolHandleStorage32(IndexType _index, VersionType _version)
            : m_index(_index),
              m_version(_version) {}

        constexpr ObjectPoolHandleStorage32(const ObjectPoolHandleStorage32&) = default;

        IndexType getIndex() const { return m_index; }
        VersionType getVersion() const { return m_version; }
    };

    struct ObjectPoolHandleStorage64 final
    {
        using KeyType = uint64_t;
        using IndexType = uint32_t;
        using VersionType = uint32_t;

        static constexpr KeyType InvalidKey = 0;
        static constexpr VersionType InvalidVersion = 0;

        union
        {
            KeyType m_key = InvalidKey;

            struct
            {
                IndexType m_index;
                VersionType m_version;
            };
        };

        constexpr ObjectPoolHandleStorage64() = default;
        constexpr ObjectPoolHandleStorage64(KeyType _key): m_key(_key) {}

        constexpr ObjectPoolHandleStorage64(IndexType _index, VersionType _version)
            : m_index(_index),
              m_version(_version) {}

        constexpr ObjectPoolHandleStorage64(const ObjectPoolHandleStorage64&) = default;

        IndexType getIndex() const { return m_index; }
        VersionType getVersion() const { return m_version; }
    };

    template <typename ObjectPoolHandleStorageType>
    struct ObjectPoolHandle final
    {
        using HandleStorageType = ObjectPoolHandleStorageType;

        using KeyType = typename ObjectPoolHandleStorageType::KeyType;
        using IndexType = typename ObjectPoolHandleStorageType::IndexType;
        using VersionType = typename ObjectPoolHandleStorageType::VersionType;

        static constexpr KeyType InvalidKey = ObjectPoolHandleStorageType::InvalidKey;
        static constexpr VersionType InvalidVersion = ObjectPoolHandleStorageType::InvalidVersion;

        HandleStorageType m_handle;

        constexpr ObjectPoolHandle() = default;
        constexpr ObjectPoolHandle(KeyType _key): m_handle(_key) {}

        constexpr ObjectPoolHandle(IndexType _index, VersionType _version)
            : m_handle(_index, _version) {}

        constexpr ObjectPoolHandle(HandleStorageType _handle): m_handle(_handle) {}
        constexpr ObjectPoolHandle(const ObjectPoolHandle&) = default;

        inline KeyType getKey() const { return m_handle.m_key; }
        inline IndexType getIndex() const { return m_handle.getIndex(); }
        inline VersionType getVersion() const { return m_handle.getVersion(); }

        static KeyType makeKey(IndexType _index, VersionType _version)
        {
            return HandleStorageType(_index, _version).m_key;
        }

        static IndexType getIndex(KeyType _key)
        {
            return HandleStorageType(_key).getIndex();
        }

        static VersionType getVersion(KeyType _key)
        {
            return HandleStorageType(_key).getVersion();
        }
    };

    using ObjectPoolHandle8 = ObjectPoolHandle<ObjectPoolHandleStorage8>;
    using ObjectPoolHandle16 = ObjectPoolHandle<ObjectPoolHandleStorage16>;
    using ObjectPoolHandle32 = ObjectPoolHandle<ObjectPoolHandleStorage32>;
    using ObjectPoolHandle64 = ObjectPoolHandle<ObjectPoolHandleStorage64>;
}
