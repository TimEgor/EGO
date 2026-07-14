#include "PluginCatalog.h"

#include <cctype>
#include <functional>
#include <iterator>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>

#include "EgoCore/Assert/Assert.h"

class ego::PluginCatalog::Storage final
{
public:
    Storage() = default;

    RegistrationID registerModules(const ModuleCollection& _modules);
    bool unregisterModules(RegistrationID _registrationID);
    void clear();

    FileName resolve(PluginType _pluginType) const;
    FileName resolve(PluginType _pluginType, std::string_view _pluginName) const;

private:
    struct NamedPluginKey final
    {
        PluginType m_type = rtti::InvalidTypeMetaInfoID;
        std::string m_name;

        bool operator==(const NamedPluginKey& _key) const = default;
    };

    struct NamedPluginKeyHash final
    {
        size_t operator()(const NamedPluginKey& _key) const;
    };

    using BindingChain = std::list<FileName>;
    using BindingIterator = BindingChain::iterator;

    struct TypeBinding final
    {
        PluginType m_type = rtti::InvalidTypeMetaInfoID;
        BindingIterator m_binding;
    };

    struct NamedBinding final
    {
        NamedPluginKey m_key;
        BindingIterator m_binding;
    };

    struct Registration final
    {
        std::vector<TypeBinding> m_typeBindings;
        std::vector<NamedBinding> m_namedBindings;
    };

    using RegistrationCollection = std::unordered_map<RegistrationID, Registration>;
    using TypeBindingIndex = std::unordered_map<PluginType, BindingChain>;
    using NamedBindingIndex = std::unordered_map<NamedPluginKey, BindingChain, NamedPluginKeyHash>;

    void addRegistration(RegistrationID _registrationID, const ModuleCollection& _modules);
    void addTypeBinding(Registration& _registration, PluginType _pluginType, const FileName& _moduleName);
    void addNamedBinding(Registration& _registration, NamedPluginKey _key, const FileName& _moduleName);
    void removeTypeBinding(const TypeBinding& _binding);
    void removeNamedBinding(const NamedBinding& _binding);

    static NamedPluginKey CreateNamedKey(PluginType _pluginType, std::string_view _pluginName);
    static std::string NormalizeName(std::string_view _pluginName);
    static bool IsPluginValid(const PluginDesc& _plugin);
    static bool IsModuleValid(const ModuleDesc& _module);
    static bool AreModulesValid(const ModuleCollection& _modules);

    mutable std::shared_mutex m_mutex;
    RegistrationCollection m_registrations;
    TypeBindingIndex m_typeBindings;
    NamedBindingIndex m_namedBindings;
    RegistrationID m_nextRegistrationID = InvalidRegistrationID + 1;
};

ego::PluginCatalog::PluginCatalog()
    : m_storage(std::make_unique<Storage>())
{
}

ego::PluginCatalog::~PluginCatalog() = default;

ego::PluginCatalog::RegistrationID ego::PluginCatalog::registerModules(const ModuleCollection& _modules)
{
    return m_storage->registerModules(_modules);
}

bool ego::PluginCatalog::unregisterModules(RegistrationID _registrationID)
{
    return m_storage->unregisterModules(_registrationID);
}

void ego::PluginCatalog::clear()
{
    m_storage->clear();
}

ego::FileName ego::PluginCatalog::resolve(PluginType _pluginType) const
{
    return m_storage->resolve(_pluginType);
}

ego::FileName ego::PluginCatalog::resolve(PluginType _pluginType, std::string_view _pluginName) const
{
    return m_storage->resolve(_pluginType, _pluginName);
}

ego::PluginCatalog::RegistrationID ego::PluginCatalog::Storage::registerModules(const ModuleCollection& _modules)
{
    if (!AreModulesValid(_modules))
    {
        return InvalidRegistrationID;
    }

    std::unique_lock lock(m_mutex);
    if (m_nextRegistrationID == InvalidRegistrationID)
    {
        return InvalidRegistrationID;
    }

    const RegistrationID registrationID = m_nextRegistrationID++;
    addRegistration(registrationID, _modules);
    return registrationID;
}

bool ego::PluginCatalog::Storage::unregisterModules(RegistrationID _registrationID)
{
    if (_registrationID == InvalidRegistrationID)
    {
        return false;
    }

    std::unique_lock lock(m_mutex);
    const auto registrationIt = m_registrations.find(_registrationID);
    if (registrationIt == m_registrations.end())
    {
        return false;
    }

    for (const TypeBinding& binding : registrationIt->second.m_typeBindings)
    {
        removeTypeBinding(binding);
    }
    for (const NamedBinding& binding : registrationIt->second.m_namedBindings)
    {
        removeNamedBinding(binding);
    }

    m_registrations.erase(registrationIt);
    return true;
}

void ego::PluginCatalog::Storage::clear()
{
    std::unique_lock lock(m_mutex);

    m_typeBindings.clear();
    m_namedBindings.clear();
    m_registrations.clear();
}

ego::FileName ego::PluginCatalog::Storage::resolve(PluginType _pluginType) const
{
    if (_pluginType == rtti::InvalidTypeMetaInfoID)
    {
        return FileName();
    }

    std::shared_lock lock(m_mutex);
    const auto bindingIt = m_typeBindings.find(_pluginType);
    return bindingIt != m_typeBindings.end() ? bindingIt->second.back() : FileName();
}

ego::FileName ego::PluginCatalog::Storage::resolve(PluginType _pluginType, std::string_view _pluginName) const
{
    if (_pluginType == rtti::InvalidTypeMetaInfoID || _pluginName.empty())
    {
        return FileName();
    }

    std::shared_lock lock(m_mutex);
    const auto bindingIt = m_namedBindings.find(CreateNamedKey(_pluginType, _pluginName));
    return bindingIt != m_namedBindings.end() ? bindingIt->second.back() : FileName();
}

void ego::PluginCatalog::Storage::addRegistration(RegistrationID _registrationID, const ModuleCollection& _modules)
{
    const auto [registrationIt, registrationInserted] = m_registrations.try_emplace(_registrationID);
    EGO_ASSERT(registrationInserted);

    Registration& registration = registrationIt->second;
    for (const ModuleDesc& module : _modules)
    {
        for (const PluginDesc& plugin : module.m_plugins)
        {
            addTypeBinding(registration, plugin.m_type, module.m_moduleName);
            addNamedBinding(registration, CreateNamedKey(plugin.m_type, plugin.m_name), module.m_moduleName);
        }
    }
}

void ego::PluginCatalog::Storage::addTypeBinding(Registration& _registration, PluginType _pluginType, const FileName& _moduleName)
{
    BindingChain& bindings = m_typeBindings[_pluginType];
    bindings.push_back(_moduleName);
    _registration.m_typeBindings.push_back({_pluginType, std::prev(bindings.end())});
}

void ego::PluginCatalog::Storage::addNamedBinding(Registration& _registration, NamedPluginKey _key, const FileName& _moduleName)
{
    BindingChain& bindings = m_namedBindings[_key];
    bindings.push_back(_moduleName);
    _registration.m_namedBindings.push_back({std::move(_key), std::prev(bindings.end())});
}

void ego::PluginCatalog::Storage::removeTypeBinding(const TypeBinding& _binding)
{
    const auto bindingIt = m_typeBindings.find(_binding.m_type);
    EGO_ASSERT(bindingIt != m_typeBindings.end());

    bindingIt->second.erase(_binding.m_binding);
    if (bindingIt->second.empty())
    {
        m_typeBindings.erase(bindingIt);
    }
}

void ego::PluginCatalog::Storage::removeNamedBinding(const NamedBinding& _binding)
{
    const auto bindingIt = m_namedBindings.find(_binding.m_key);
    EGO_ASSERT(bindingIt != m_namedBindings.end());

    bindingIt->second.erase(_binding.m_binding);
    if (bindingIt->second.empty())
    {
        m_namedBindings.erase(bindingIt);
    }
}

size_t ego::PluginCatalog::Storage::NamedPluginKeyHash::operator()(const NamedPluginKey& _key) const
{
    const size_t typeHash = std::hash<PluginType>()(_key.m_type);
    const size_t nameHash = std::hash<std::string>()(_key.m_name);
    return typeHash ^ (nameHash + 0x9e3779b9 + (typeHash << 6) + (typeHash >> 2));
}

ego::PluginCatalog::Storage::NamedPluginKey ego::PluginCatalog::Storage::CreateNamedKey(PluginType _pluginType, std::string_view _pluginName)
{
    NamedPluginKey key;
    key.m_type = _pluginType;
    key.m_name = NormalizeName(_pluginName);
    return key;
}

std::string ego::PluginCatalog::Storage::NormalizeName(std::string_view _pluginName)
{
    std::string name;
    name.reserve(_pluginName.size());
    for (const char character : _pluginName)
    {
        name.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    }

    return name;
}

bool ego::PluginCatalog::Storage::IsPluginValid(const PluginDesc& _plugin)
{
    return _plugin.m_type != rtti::InvalidTypeMetaInfoID && !_plugin.m_name.empty();
}

bool ego::PluginCatalog::Storage::IsModuleValid(const ModuleDesc& _module)
{
    if (!_module.m_moduleName || _module.m_plugins.empty())
    {
        return false;
    }

    for (const PluginDesc& plugin : _module.m_plugins)
    {
        if (!IsPluginValid(plugin))
        {
            return false;
        }
    }

    return true;
}

bool ego::PluginCatalog::Storage::AreModulesValid(const ModuleCollection& _modules)
{
    for (const ModuleDesc& module : _modules)
    {
        if (!IsModuleValid(module))
        {
            return false;
        }
    }

    return true;
}
