#pragma once

#include <mutex>
#include <typeinfo>
#include <unordered_map>

#include "EgoCore/Hash/CRC32.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego
{
    class PluginModuleBinding
    {
    public:
        PluginModuleBinding() = default;
        virtual ~PluginModuleBinding() = default;
    };

    EGO_POINTER(PluginModuleBinding);

    template <typename T>
    class TypedPluginModuleBinding final : public PluginModuleBinding
    {
    public:
        explicit TypedPluginModuleBinding(const SharedPointer<T>& _instance)
            : m_instance(_instance)
        {}

        SharedPointer<T> getInstance() const
        {
            return m_instance;
        }

    private:
        SharedPointer<T> m_instance;
    };

    class PluginModuleBindingBridge final
    {
    public:
        using BindingTypeID = uint32_t;
        static constexpr BindingTypeID InvalidBindingTypeID = 0;

        PluginModuleBindingBridge() = default;

        template <typename T>
        void addBinding(const SharedPointer<T>& _instance)
        {
            addBinding(
                PluginModuleBindingPointer(new TypedPluginModuleBinding<T>(_instance)),
                GetBindingType<T>()
            );
        }

        template <typename T>
        SharedPointer<T> getBinding() const
        {
            PluginModuleBindingPointer binding = getBinding(GetBindingType<T>());
            if (!binding)
            {
                return nullptr;
            }

            return static_cast<TypedPluginModuleBinding<T>*>(binding.get())->getInstance();
        }

        template <typename T>
        bool removeBinding()
        {
            return removeBinding(GetBindingType<T>());
        }

        void clear();

    private:
        template <typename T>
        static constexpr BindingTypeID GetBindingType()
        {
            return Crc32(typeid(T).name());
        }

        void addBinding(const PluginModuleBindingPointer& _binding, BindingTypeID _id);
        PluginModuleBindingPointer getBinding(BindingTypeID _id) const;
        bool removeBinding(BindingTypeID _id);

        mutable std::mutex m_lock;

        std::unordered_map<BindingTypeID, PluginModuleBindingPointer> m_bindings;
    };
}
