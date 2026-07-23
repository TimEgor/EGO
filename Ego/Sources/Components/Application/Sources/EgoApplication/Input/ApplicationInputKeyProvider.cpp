#include "ApplicationInputKeyProvider.h"

ego::application::ApplicationInputKeyProvider::ApplicationInputKeyProvider(InputDeviceController& _inputDeviceController)
    : m_inputDeviceController(_inputDeviceController)
{
}

ego::application::ApplicationInputKeyProvider::~ApplicationInputKeyProvider() = default;

void ego::application::ApplicationInputKeyProvider::updateDevices()
{
    m_inputDeviceController.update();
}

const ego::InputKeyProvider::DeviceCollection& ego::application::ApplicationInputKeyProvider::getDevices() const
{
    return m_inputDeviceController.getDevices();
}
