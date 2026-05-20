#pragma once

#include "EgoCore/Patterns/Singleton.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEngine/Engine.h"

#include "Demo.h"
#include "PluginController/DemoPluginController.h"

namespace ego::demo
{
    struct DemoControllerInitData final
    {
        engine::EngineInitData m_engineInitData;
        FileName m_demoPluginModuleName;
    };

    class DemoController final
    {
    public:
        DemoController() = default;
        ~DemoController() { release(); }

        bool init(const DemoControllerInitData& _initData);
        bool init(const engine::EngineInitData& _initData);
        void release();

        bool loadDemo(const FileName& _moduleName);
        void run();

        DemoPointer getCurrentDemo() const;
        const Demo& getCurrentDemoRef() const;
        Demo& getCurrentDemoRef();

    private:
        engine::EnginePointer m_engine = nullptr;

        DemoPluginControllerPointer m_pluginController = nullptr;
        DemoPluginPointer m_currentDemoPlugin = nullptr;
        DemoPointer m_currentDemo = nullptr;
    };

    EGO_POINTER(DemoController);
    EGO_WEAK_POINTER(DemoController);

    class DemoControllerCore final : public Singleton<DemoControllerCore>
    {
    public:
        DemoControllerCore() = default;

        DemoControllerPointer getController() const;
        void init(const DemoControllerPointer& _controller);
        void release();

    private:
        DemoControllerPointer m_controller = nullptr;
    };

    DemoController& GetDemoController();
}
