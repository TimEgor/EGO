#include "RuntimeInputKeyProvider.h"

ego::runtime::RuntimeInputKeyProvider::RuntimeInputKeyProvider(InputDeviceController& _inputDeviceController)
    : m_inputDeviceController(_inputDeviceController)
{
}

ego::runtime::RuntimeInputKeyProvider::~RuntimeInputKeyProvider() = default;

void ego::runtime::RuntimeInputKeyProvider::updateDevices()
{
    m_inputDeviceController.update();
}

const ego::InputKeyProvider::DeviceCollection& ego::runtime::RuntimeInputKeyProvider::getDevices() const
{
    return m_inputDeviceController.getDevices();
}
