#pragma once

#include "EgoCore/Patterns/Singleton.h"

#include "EgoEngine/Engine.h"

#include "PluginController/DemoPluginController.h"

namespace ego::demo
{
    class DemoController final
    {
    public:
        DemoController() = default;

        bool init(const engine::EngineInitData& _initData);
        void release();

        void run();

    private:
        engine::Engine* m_engine = nullptr;

        DemoPluginController* m_pluginController = nullptr;
    };

    class DemoControllerCore final : public Singleton<DemoControllerCore>
    {
    public:
        DemoControllerCore() = default;

        DemoController& getController() const;
        void init(DemoController* _controller);

    private:
        DemoController* m_controller;
    };

    DemoController& GetDemoController();
}
