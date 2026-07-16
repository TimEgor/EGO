#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>

namespace ego
{
    template <typename HandleKeyType, typename HandleIndexType, typename HandleVersionType, uint8_t HandleIndexBits>
    struct ObjectPoolHandleStorageBase
    {
        using KeyType = HandleKeyType;
        using IndexType = HandleIndexType;
        using VersionType = HandleVersionType;

        static_assert(std::is_unsigned_v<KeyType>);
        static_assert(std::is_unsigned_v<IndexType>);
        static_assert(std::is_unsigned_v<VersionType>);

        static constexpr uint8_t KeyBits = static_cast<uint8_t>(sizeof(KeyType) * 8);
        static constexpr uint8_t IndexBits = HandleIndexBits;
        static constexpr uint8_t VersionBits = KeyBits - IndexBits;

        static_assert(IndexBits > 0);
        static_assert(IndexBits < KeyBits);
        static_assert(VersionBits > 0);

        static constexpr KeyType InvalidKey = 0;
        static constexpr VersionType InvalidVersion = 0;

        static constexpr KeyType IndexMask = (KeyType(1) << IndexBits) - 1;
        static constexpr KeyType VersionMask = (KeyType(1) << VersionBits) - 1;
        static constexpr IndexType MaxIndex = static_cast<IndexType>(IndexMask);
        static constexpr VersionType MaxVersion = static_cast<VersionType>(VersionMask);

        KeyType m_key = InvalidKey;

        constexpr ObjectPoolHandleStorageBase() noexcept = default;
        constexpr ObjectPoolHandleStorageBase(KeyType _key) noexcept
            : m_key(_key)
        {
        }

        constexpr ObjectPoolHandleStorageBase(IndexType _index, VersionType _version) noexcept
            : m_key(makeKey(_index, _version))
        {
        }

        constexpr ObjectPoolHandleStorageBase(const ObjectPoolHandleStorageBase&) noexcept = default;
        constexpr ObjectPoolHandleStorageBase& operator=(const ObjectPoolHandleStorageBase&) noexcept = default;

        static constexpr KeyType makeKey(IndexType _index, VersionType _version) noexcept
        {
            const KeyType index = static_cast<KeyType>(_index) & IndexMask;
            const KeyType version = static_cast<KeyType>(_version) & VersionMask;
            return index | (version << IndexBits);
        }

        static constexpr IndexType getIndex(KeyType _key) noexcept
        {
            return static_cast<IndexType>(_key & IndexMask);
        }

        static constexpr VersionType getVersion(KeyType _key) noexcept
        {
            return static_cast<VersionType>((_key >> IndexBits) & VersionMask);
        }

        constexpr IndexType getIndex() const noexcept
        {
            return getIndex(m_key);
        }
        constexpr VersionType getVersion() const noexcept
        {
            return getVersion(m_key);
        }
    };

    struct ObjectPoolHandleStorage8 final : ObjectPoolHandleStorageBase<uint8_t, uint8_t, uint8_t, 4>
    {
        using BaseType = ObjectPoolHandleStorageBase<uint8_t, uint8_t, uint8_t, 4>;
        using BaseType::BaseType;

        constexpr ObjectPoolHandleStorage8() noexcept = default;
        constexpr ObjectPoolHandleStorage8(const ObjectPoolHandleStorage8&) noexcept = default;
        constexpr ObjectPoolHandleStorage8& operator=(const ObjectPoolHandleStorage8&) noexcept = default;
    };

    struct ObjectPoolHandleStorage16 final : ObjectPoolHandleStorageBase<uint16_t, uint8_t, uint8_t, 8>
    {
        using BaseType = ObjectPoolHandleStorageBase<uint16_t, uint8_t, uint8_t, 8>;
        using BaseType::BaseType;

        constexpr ObjectPoolHandleStorage16() noexcept = default;
        constexpr ObjectPoolHandleStorage16(const ObjectPoolHandleStorage16&) noexcept = default;
        constexpr ObjectPoolHandleStorage16& operator=(const ObjectPoolHandleStorage16&) noexcept = default;
    };

    struct ObjectPoolHandleStorage32 final : ObjectPoolHandleStorageBase<uint32_t, uint16_t, uint16_t, 16>
    {
        using BaseType = ObjectPoolHandleStorageBase<uint32_t, uint16_t, uint16_t, 16>;
        using BaseType::BaseType;

        constexpr ObjectPoolHandleStorage32() noexcept = default;
        constexpr ObjectPoolHandleStorage32(const ObjectPoolHandleStorage32&) noexcept = default;
        constexpr ObjectPoolHandleStorage32& operator=(const ObjectPoolHandleStorage32&) noexcept = default;
    };

    struct ObjectPoolHandleStorage64 final : ObjectPoolHandleStorageBase<uint64_t, uint32_t, uint32_t, 32>
    {
        using BaseType = ObjectPoolHandleStorageBase<uint64_t, uint32_t, uint32_t, 32>;
        using BaseType::BaseType;

        constexpr ObjectPoolHandleStorage64() noexcept = default;
        constexpr ObjectPoolHandleStorage64(const ObjectPoolHandleStorage64&) noexcept = default;
        constexpr ObjectPoolHandleStorage64& operator=(const ObjectPoolHandleStorage64&) noexcept = default;
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
        static constexpr IndexType MaxIndex = ObjectPoolHandleStorageType::MaxIndex;
        static constexpr VersionType MaxVersion = ObjectPoolHandleStorageType::MaxVersion;

        HandleStorageType m_handle;

        constexpr ObjectPoolHandle() noexcept = default;
        constexpr ObjectPoolHandle(KeyType _key) noexcept
            : m_handle(_key)
        {
        }

        constexpr ObjectPoolHandle(IndexType _index, VersionType _version) noexcept
            : m_handle(_index, _version)
        {
        }

        constexpr ObjectPoolHandle(HandleStorageType _handle) noexcept
            : m_handle(_handle)
        {
        }

        constexpr ObjectPoolHandle(const ObjectPoolHandle&) noexcept = default;
        constexpr ObjectPoolHandle& operator=(const ObjectPoolHandle&) noexcept = default;

        constexpr KeyType getKey() const noexcept
        {
            return m_handle.m_key;
        }
        constexpr IndexType getIndex() const noexcept
        {
            return m_handle.getIndex();
        }
        constexpr VersionType getVersion() const noexcept
        {
            return m_handle.getVersion();
        }
        constexpr bool isValid() const noexcept
        {
            return getKey() != InvalidKey;
        }

        constexpr explicit operator bool() const noexcept
        {
            return isValid();
        }

        friend constexpr bool operator==(ObjectPoolHandle _left, ObjectPoolHandle _right) noexcept
        {
            return _left.getKey() == _right.getKey();
        }

        friend constexpr bool operator!=(ObjectPoolHandle _left, ObjectPoolHandle _right) noexcept
        {
            return !(_left == _right);
        }

        static constexpr bool canRepresentIndex(IndexType _index) noexcept
        {
            return _index <= MaxIndex;
        }

        static constexpr bool canRepresentVersion(VersionType _version) noexcept
        {
            return _version <= MaxVersion;
        }

        static constexpr KeyType makeKey(IndexType _index, VersionType _version) noexcept
        {
            return HandleStorageType::makeKey(_index, _version);
        }

        static constexpr IndexType getIndex(KeyType _key) noexcept
        {
            return HandleStorageType::getIndex(_key);
        }

        static constexpr VersionType getVersion(KeyType _key) noexcept
        {
            return HandleStorageType::getVersion(_key);
        }
    };

    static_assert(sizeof(ObjectPoolHandleStorage8) == sizeof(ObjectPoolHandleStorage8::KeyType));
    static_assert(sizeof(ObjectPoolHandleStorage16) == sizeof(ObjectPoolHandleStorage16::KeyType));
    static_assert(sizeof(ObjectPoolHandleStorage32) == sizeof(ObjectPoolHandleStorage32::KeyType));
    static_assert(sizeof(ObjectPoolHandleStorage64) == sizeof(ObjectPoolHandleStorage64::KeyType));

    using ObjectPoolHandle8 = ObjectPoolHandle<ObjectPoolHandleStorage8>;
    using ObjectPoolHandle16 = ObjectPoolHandle<ObjectPoolHandleStorage16>;
    using ObjectPoolHandle32 = ObjectPoolHandle<ObjectPoolHandleStorage32>;
    using ObjectPoolHandle64 = ObjectPoolHandle<ObjectPoolHandleStorage64>;

    static_assert(sizeof(ObjectPoolHandle8) == sizeof(ObjectPoolHandle8::KeyType));
    static_assert(sizeof(ObjectPoolHandle16) == sizeof(ObjectPoolHandle16::KeyType));
    static_assert(sizeof(ObjectPoolHandle32) == sizeof(ObjectPoolHandle32::KeyType));
    static_assert(sizeof(ObjectPoolHandle64) == sizeof(ObjectPoolHandle64::KeyType));

    static_assert(ObjectPoolHandle8::makeKey(0x0F, 0x0F) == 0xFF);
    static_assert(ObjectPoolHandle16::makeKey(0xFF, 0xFF) == 0xFFFF);
    static_assert(ObjectPoolHandle32::makeKey(0xFFFF, 0xFFFF) == 0xFFFFFFFF);
} // namespace ego

namespace std
{
    template <typename ObjectPoolHandleStorageType>
    struct hash<ego::ObjectPoolHandle<ObjectPoolHandleStorageType>>
    {
        size_t operator()(ego::ObjectPoolHandle<ObjectPoolHandleStorageType> _handle) const noexcept
        {
            using KeyType = typename ego::ObjectPoolHandle<ObjectPoolHandleStorageType>::KeyType;
            return hash<KeyType>()(_handle.getKey());
        }
    };
} // namespace std
