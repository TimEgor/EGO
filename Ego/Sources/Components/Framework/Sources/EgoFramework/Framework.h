#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/Singleton.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEngine/Engine.h"

#include "EgoPlugin/PluginController.h"

#include "GameLogic.h"
#include "Plugin/GameLogicPlugin.h"
#include "Plugin/GameLogicPluginController.h"
#include "Project.h"

namespace ego::framework
{
    class FrameworkEngine;

    class Framework : public NonCopyable
    {
        friend class FrameworkEngine;

    public:
        struct InitData final
        {
            engine::Engine::InitData m_engineInitData;
            ProjectPointer m_project = nullptr;
        };

        Framework() = default;
        virtual ~Framework() { release(); }

        bool init(const InitData& _initData);
        void release();

        void run();
        bool loadGameLogic(const FileName& _moduleName);

        PluginController& getPluginController() const;
        engine::Engine& getEngine() const;
        ProjectPointer getProject() const;
        GameLogic& getCurrentGameLogic() const;

    private:
        bool initPluginController();
        void releasePluginController();
        bool initEngine(const InitData& _initData);
        FileName selectProjectGameLogicPluginModule() const;
        void updateCurrentGameLogic(float _deltaTime);

        PluginControllerPointer m_pluginController = nullptr;

        engine::EnginePointer m_engine = nullptr;
        ProjectPointer m_project = nullptr;

        GameLogicPluginControllerPointer m_gameLogicPluginController = nullptr;
        GameLogicPluginPointer m_currentGameLogicPlugin = nullptr;
        GameLogicPointer m_currentGameLogic = nullptr;
    };

    EGO_POINTER(Framework);
    EGO_WEAK_POINTER(Framework);

    class FrameworkCore final : public Singleton<FrameworkCore>
    {
    public:
        FrameworkCore() = default;

        bool init(const FrameworkPointer& _framework);
        void release();
        FrameworkPointer getFramework() const;

    private:
        FrameworkPointer m_framework = nullptr;
    };

    Framework& GetFramework();
}

