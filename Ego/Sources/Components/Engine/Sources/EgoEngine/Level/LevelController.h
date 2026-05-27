#pragma once

#include "Level.h"

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include <unordered_map>

namespace ego
{
    class LevelController final : public NonCopyable
    {
    public:
        class LevelControllerAccessor final : public NonInstanceable
        {
            friend struct LevelDeleter;

            static bool RemoveLevel(LevelController& _controller, LevelID _levelID);
        };

        LevelController() = default;
        ~LevelController();

        bool init();
        void release();

        bool isInitialized() const;

        LevelPointer createLevel();

        LevelPointer getLevel(LevelID _levelID) const;

        LevelPointer getActiveLevel() const;
        bool setActiveLevel(LevelID _levelID);
        void clearActiveLevel();

    private:
        using LevelMap = std::unordered_map<LevelID, LevelWeakPointer>;

        LevelID allocateLevelID();
        bool removeLevel(LevelID _levelID);

        LevelID m_nextLevelID = FirstLevelID;
        LevelMap m_levels;
        LevelWeakPointer m_activeLevel;
        bool m_isInitialized = false;
    };

    EGO_POINTER(LevelController);
    EGO_WEAK_POINTER(LevelController);
}
