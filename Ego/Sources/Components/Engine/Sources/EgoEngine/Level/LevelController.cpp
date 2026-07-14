#include "LevelController.h"

#include "EgoCore/Assert/Assert.h"

ego::LevelController::~LevelController()
{
    release();
}

bool ego::LevelController::LevelControllerAccessor::RemoveLevel(LevelController& _controller, LevelID _levelID)
{
    return _controller.removeLevel(_levelID);
}

bool ego::LevelController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    m_nextLevelID = FirstLevelID;
    m_isInitialized = true;

    return true;
}

void ego::LevelController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    LevelMap levels = m_levels;

    m_activeLevel.reset();
    m_levels.clear();

    for (const auto& levelEntry : levels)
    {
        const LevelPointer level = levelEntry.second.lock();
        if (level)
        {
            level->clear();
        }
    }

    m_nextLevelID = FirstLevelID;
    m_isInitialized = false;
}

bool ego::LevelController::isInitialized() const
{
    return m_isInitialized;
}

ego::LevelPointer ego::LevelController::createLevel()
{
    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL_MESSAGE("LevelController isn't initialized.");
        return nullptr;
    }

    const LevelID levelID = allocateLevelID();
    LevelPointer level(new Level(levelID), LevelDeleter(weakFromThis()));
    if (!level)
    {
        return nullptr;
    }

    m_levels.emplace(levelID, LevelWeakPointer(level));
    return level;
}

ego::LevelPointer ego::LevelController::getLevel(LevelID _levelID) const
{
    const auto levelIt = m_levels.find(_levelID);
    if (levelIt == m_levels.end())
    {
        return nullptr;
    }

    return levelIt->second.lock();
}

ego::LevelPointer ego::LevelController::getActiveLevel() const
{
    return m_activeLevel.lock();
}

bool ego::LevelController::setActiveLevel(LevelID _levelID)
{
    const LevelPointer level = getLevel(_levelID);
    if (!level)
    {
        return false;
    }

    m_activeLevel = level;
    return true;
}

void ego::LevelController::clearActiveLevel()
{
    m_activeLevel.reset();
}

ego::LevelID ego::LevelController::allocateLevelID()
{
    LevelID levelID = InvalidLevelID;

    do
    {
        levelID = m_nextLevelID;
        ++m_nextLevelID;

        if (m_nextLevelID == InvalidLevelID)
        {
            m_nextLevelID = FirstLevelID;
        }
    } while (levelID == InvalidLevelID || m_levels.find(levelID) != m_levels.end());

    return levelID;
}

bool ego::LevelController::removeLevel(LevelID _levelID)
{
    if (_levelID == InvalidLevelID)
    {
        return false;
    }

    const LevelPointer activeLevel = m_activeLevel.lock();
    if (activeLevel && activeLevel->getID() == _levelID)
    {
        m_activeLevel.reset();
    }

    const auto levelIt = m_levels.find(_levelID);
    if (levelIt == m_levels.end())
    {
        return false;
    }

    m_levels.erase(levelIt);
    return true;
}
