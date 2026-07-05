#include "InputDeviceController.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

bool ego::InputDeviceController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    for (ProviderData& providerData : m_providers)
    {
        EGO_CHECK_RETURN_CALL_FALSE(addProviderDevices(providerData), release());
    }

    rebuildDeviceIndex();
    m_isInitialized = true;

    return true;
}

void ego::InputDeviceController::release()
{
    if (!m_isInitialized && m_providers.empty() && m_devices.empty() && m_deviceIndex.empty())
    {
        return;
    }

    m_devices.clear();
    m_deviceIndex.clear();
    for (ProviderData& providerData : m_providers)
    {
        providerData.m_devices.clear();
    }

    m_isInitialized = false;
}

bool ego::InputDeviceController::addProvider(const InputDeviceProviderPointer& _provider)
{
    EGO_CHECK_RETURN_FALSE(_provider);

    if (hasProvider(_provider))
    {
        return true;
    }

    ProviderData providerData;
    providerData.m_provider = _provider;
    if (m_isInitialized)
    {
        EGO_CHECK_RETURN_FALSE(addProviderDevices(providerData));
    }

    m_providers.push_back(providerData);
    rebuildDeviceIndex();

    return true;
}

bool ego::InputDeviceController::removeProvider(const InputDeviceProviderPointer& _provider)
{
    EGO_ASSERT(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(_provider);

    for (ProviderCollection::iterator providerIt = m_providers.begin(); providerIt != m_providers.end(); ++providerIt)
    {
        if (providerIt->m_provider.get() == _provider.get())
        {
            m_providers.erase(providerIt);
            rebuildDeviceIndex();
            return true;
        }
    }

    return false;
}

void ego::InputDeviceController::update()
{
    EGO_CHECK_RETURN(m_isInitialized);

    for (ProviderData& providerData : m_providers)
    {
        for (const InputDevicePointer& device : providerData.m_devices)
        {
            if (!device)
            {
                continue;
            }

            device->storePreviousValues();
            device->update();
        }
    }
}

const ego::InputDeviceController::DeviceCollection& ego::InputDeviceController::getDevices() const
{
    return m_devices;
}

const ego::InputDeviceController::DeviceCollection& ego::InputDeviceController::getDevices(InputDeviceType _deviceType) const
{
    static const DeviceCollection EmptyDevices;

    const DeviceCollectionByType::const_iterator devicesIt = m_deviceIndex.find(_deviceType);
    if (devicesIt == m_deviceIndex.end())
    {
        return EmptyDevices;
    }

    return devicesIt->second;
}

bool ego::InputDeviceController::isInitialized() const
{
    return m_isInitialized;
}

bool ego::InputDeviceController::hasProvider(const InputDeviceProviderPointer& _provider) const
{
    for (const ProviderData& providerData : m_providers)
    {
        if (providerData.m_provider.get() == _provider.get())
        {
            return true;
        }
    }

    return false;
}

bool ego::InputDeviceController::addProviderDevices(ProviderData& _providerData)
{
    EGO_CHECK_RETURN_FALSE(_providerData.m_provider);

    _providerData.m_devices = _providerData.m_provider->createDevices();
    for (const InputDevicePointer& device : _providerData.m_devices)
    {
        EGO_CHECK_RETURN_FALSE(device);
    }

    return true;
}

void ego::InputDeviceController::rebuildDeviceIndex()
{
    m_devices.clear();
    m_deviceIndex.clear();

    for (const ProviderData& providerData : m_providers)
    {
        for (const InputDevicePointer& device : providerData.m_devices)
        {
            addDeviceToIndex(device);
        }
    }
}

void ego::InputDeviceController::addDeviceToIndex(const InputDevicePointer& _device)
{
    EGO_CHECK_RETURN(_device);

    const InputDeviceType deviceType = _device->getType();
    EGO_CHECK_RETURN(deviceType != InvalidInputDeviceType);

    m_devices.push_back(_device);
    m_deviceIndex[deviceType].push_back(_device);
}
