#include "LevelComponent.h"

#include <utility>

ego::LevelComponent::LevelComponent(LevelID _levelID)
    : m_levelID(_levelID)
{
}

ego::NameComponent::NameComponent(std::string _name)
    : m_name(std::move(_name))
{
}
