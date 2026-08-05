#include "CameraComponent.h"

ego::render::CameraComponent::CameraComponent(float _verticalFieldOfView, float _nearPlane, float _farPlane)
    : m_verticalFieldOfView(_verticalFieldOfView),
      m_nearPlane(_nearPlane),
      m_farPlane(_farPlane)
{
}
