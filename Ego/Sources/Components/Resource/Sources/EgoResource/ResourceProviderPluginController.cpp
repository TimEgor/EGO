#include "ResourceProviderPluginController.h"

#include <algorithm>
#include <utility>

#include "EgoCore/Assert/Assert.h"

#include "EgoPlugin/PluginCatalog.h"
#include "EgoPlugin/PluginSubsystem.h"
#include "EgoPlugin/PluginController.h"

#include "ResourceSubsystem.h"
#include "ResourceController.h"

void ego::ResourceProviderPluginController::release()
{
    if (m_providers.empty())
    {
        return;
    }

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    EGO_ASSERT(resourceSubsystem);
    if (!resourceSubsystem)
    {
        return;
    }

    ResourceController& resourceController = resourceSubsystem->getResourceController();
    resourceController.waitAllLoading();

    while (!m_providers.empty())
    {
        unregisterProviderRegistrations(resourceController, m_providers.back().m_registrations);
        m_providers.pop_back();
    }
}

bool ego::ResourceProviderPluginController::registerProvider(const std::string& _providerName)
{
    if (_providerName.empty() || findProvider(_providerName) != m_providers.end())
    {
        return false;
    }

    const PluginSubsystemPointer pluginSubsystem = GetPluginSubsystemPointer();
    const PluginControllerPointer pluginController = GetPluginControllerPointer();
    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    if (!pluginSubsystem || !pluginController || !resourceSubsystem)
    {
        return false;
    }

    const FileName providerModuleName = pluginSubsystem->getPluginCatalog().getModulePath(ResourceProviderPlugin::GetPluginType(), _providerName.c_str());
    if (!providerModuleName)
    {
        return false;
    }

    const ResourceProviderPluginPointer plugin = pluginController->loadPlugin<ResourceProviderPlugin>(providerModuleName);
    if (!plugin)
    {
        return false;
    }

    ResourceProviderPlugin::RegistrationCollection registrations;
    if (!plugin->createRegistrations(registrations) || registrations.empty())
    {
        return false;
    }

    ResourceController& resourceController = resourceSubsystem->getResourceController();
    if (!registerProviderRegistrations(resourceController, registrations))
    {
        return false;
    }

    Provider provider;
    provider.m_name = _providerName;
    provider.m_plugin = plugin;
    provider.m_registrations = std::move(registrations);
    m_providers.push_back(std::move(provider));

    return true;
}

bool ego::ResourceProviderPluginController::unregisterProvider(const std::string& _providerName)
{
    const ProviderCollection::iterator providerIt = findProvider(_providerName);
    if (providerIt == m_providers.end())
    {
        return false;
    }

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    if (!resourceSubsystem)
    {
        return false;
    }

    ResourceController& resourceController = resourceSubsystem->getResourceController();
    resourceController.waitAllLoading();
    unregisterProviderRegistrations(resourceController, providerIt->m_registrations);
    m_providers.erase(providerIt);

    return true;
}

ego::ResourceProviderPluginController::ProviderCollection::iterator ego::ResourceProviderPluginController::findProvider(const std::string& _providerName)
{
    return std::find_if(
        m_providers.begin(),
        m_providers.end(),
        [&_providerName](const Provider& _provider)
        {
            return _provider.m_name == _providerName;
        });
}

bool ego::ResourceProviderPluginController::registerProviderRegistrations(
    ResourceController& _resourceController,
    const ResourceProviderPlugin::RegistrationCollection& _registrations)
{
    ResourceProviderPlugin::RegistrationCollection registeredRegistrations;
    for (const ResourceProviderPlugin::Registration& registration : _registrations)
    {
        if (!_resourceController.addResourceProvider(registration.m_extension, registration.m_provider))
        {
            unregisterProviderRegistrations(_resourceController, registeredRegistrations);
            return false;
        }

        registeredRegistrations.push_back(registration);
    }

    return true;
}

void ego::ResourceProviderPluginController::unregisterProviderRegistrations(
    ResourceController& _resourceController,
    const ResourceProviderPlugin::RegistrationCollection& _registrations)
{
    for (auto registrationIt = _registrations.rbegin(); registrationIt != _registrations.rend(); ++registrationIt)
    {
        _resourceController.removeResourceProvider(registrationIt->m_extension, registrationIt->m_provider);
    }
}
