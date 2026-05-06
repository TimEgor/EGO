#pragma once

#include <mutex>
#include <typeinfo>
#include <unordered_map>

#include "EgoCore/Hash/CRC32.h"

namespace ego
{
    class PluginModuleBindingBridge final
    {
    public:
        using BindingTypeID = uint32_t;
        static constexpr BindingTypeID InvalidBindingTypeID = 0;

        PluginModuleBindingBridge() = default;

        template <typename T>
        void addBinding(T* _instance)
        {
            addBinding(_instance, GetBindingType<T>());
        }

        template <typename T>
        T* getBinding() const
        {
            return static_cast<T*>(getBinding(GetBindingType<T>()));
        }

    private:
        template <typename T>
        static constexpr BindingTypeID GetBindingType()
        {
            return Crc32(typeid(T).name());
        }

        void addBinding(void* _instance, BindingTypeID _id);
        void* getBinding(BindingTypeID _id) const;

        mutable std::mutex m_lock;

        std::unordered_map<BindingTypeID, void*> m_bindings;
    };
}
