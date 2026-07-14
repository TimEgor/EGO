#pragma once

#include <vector>

#include "EgoPlugin/Plugin.h"

#include "ResourceProvider.h"

namespace ego
{
    class ResourceProviderPlugin : public Plugin
    {
    public:
        struct Registration final
        {
            FileName m_extension;
            ResourceProviderPointer m_provider = nullptr;
        };

        using RegistrationCollection = std::vector<Registration>;

        ResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual bool createRegistrations(RegistrationCollection& _registrations) = 0;

        EGO_PLUGIN(ResourceProviderPlugin, Plugin);
    };

    EGO_POINTER(ResourceProviderPlugin);
} // namespace ego
