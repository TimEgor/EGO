#include "NameComponent.h"

#include <utility>

ego::NameComponent::NameComponent(std::string _name)
    : m_name(std::move(_name))
{
}
