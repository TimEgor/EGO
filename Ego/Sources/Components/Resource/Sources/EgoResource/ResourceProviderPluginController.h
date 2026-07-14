#pragma once

#include <string>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "ResourceProviderPlugin.h"

namespace ego
{
    class ResourceController;

    class ResourceProviderPluginController final : public NonCopyable
    {
    public:
        ResourceProviderPluginController() = default;
        ~ResourceProviderPluginController() override = default;

        void release();

        bool registerProvider(const std::string& _providerName);
        bool unregisterProvider(const std::string& _providerName);

    private:
        struct Provider final
        {
            std::string m_name;
            ResourceProviderPluginPointer m_plugin = nullptr;
            ResourceProviderPlugin::RegistrationCollection m_registrations;
        };

        using ProviderCollection = std::vector<Provider>;

        ProviderCollection::iterator findProvider(const std::string& _providerName);
        bool registerProviderRegistrations(ResourceController& _resourceController, const ResourceProviderPlugin::RegistrationCollection& _registrations);
        void unregisterProviderRegistrations(ResourceController& _resourceController, const ResourceProviderPlugin::RegistrationCollection& _registrations);

        ProviderCollection m_providers;
    };
} // namespace ego
