#pragma once

#include <type_traits>
#include <unordered_map>
#include <vector>

#include "EgoCore/Reference/Pointer.h"
#include "InputDeviceProvider.h"

namespace ego
{
    class InputDeviceController final
    {
    public:
        using DeviceCollection = std::vector<InputDevicePointer>;

        InputDeviceController() = default;
        ~InputDeviceController()
        {
            release();
        }

        bool init();
        void release();

        bool addProvider(const InputDeviceProviderPointer& _provider);
        bool removeProvider(const InputDeviceProviderPointer& _provider);

        void update();

        const DeviceCollection& getDevices() const;
        const DeviceCollection& getDevices(InputDeviceType _deviceType) const;

        template <typename TDevice>
        const DeviceCollection& getDevices() const
        {
            static_assert(std::is_base_of_v<InputDevice, TDevice>);
            return getDevices(TDevice::GetMetaInfoID());
        }

        bool isInitialized() const;

    private:
        struct ProviderData final
        {
            InputDeviceProviderPointer m_provider;
            DeviceCollection m_devices;
        };

        using ProviderCollection = std::vector<ProviderData>;
        using DeviceCollectionByType = std::unordered_map<InputDeviceType, DeviceCollection>;

        bool hasProvider(const InputDeviceProviderPointer& _provider) const;
        bool addProviderDevices(ProviderData& _providerData);
        void rebuildDeviceIndex();
        void addDeviceToIndex(const InputDevicePointer& _device);

        ProviderCollection m_providers;
        DeviceCollection m_devices;
        DeviceCollectionByType m_deviceIndex;
        bool m_isInitialized = false;
    };

    EGO_POINTER(InputDeviceController);
} // namespace ego
