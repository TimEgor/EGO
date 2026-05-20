#include "EgoCore/FileName/FileNameUtils.h"

#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginController.h"

#include "EgoDemoFramework/DemoController.h"

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
	ego::demo::DemoControllerInitData initData;
	initData.m_engineInitData.m_nativeInstanceHandle = _hInstance;

	ego::ArgParser argParser;
	argParser.addOptionValue("--platform", initData.m_engineInitData.m_platformPluginModuleName);
	argParser.addOptionValue("--renderHardware", initData.m_engineInitData.m_renderHardwarePluginModuleName);
	argParser.addOptionValue("--demo", initData.m_demoPluginModuleName);

	argParser.parse(__argc, __argv);

    ego::PluginControllerPointer pluginController = new ego::PluginController();
    EGO_CHECK_RETURN_VALUE(ego::PluginControllerCore::GetInstance().init(pluginController), 1)
    EGO_CHECK_RETURN_VALUE(pluginController && pluginController->init(), 1);

	if (initData.m_engineInitData.m_platformPluginModuleName.empty())
	{
		initData.m_engineInitData.m_platformPluginModuleName = pluginController->selectPluginModule("platform");
	}

	if (initData.m_engineInitData.m_renderHardwarePluginModuleName.empty())
	{
		initData.m_engineInitData.m_renderHardwarePluginModuleName = pluginController->selectPluginModule("RHI");
	}

	if (initData.m_demoPluginModuleName.empty())
	{
		initData.m_demoPluginModuleName = pluginController->selectPluginModule("demo");
	}

	ego::demo::DemoControllerPointer demoController = new ego::demo::DemoController();
	ego::demo::DemoControllerCore::GetInstance().init(demoController);

	EGO_CHECK_RETURN_VALUE(demoController->init(initData), 10);

	demoController->run();

	ego::demo::DemoControllerCore::GetInstance().release();
	EGO_SAFE_RESET_POINTER_WITH_RELEASING(demoController);

    ego::PluginControllerCore::GetInstance().release();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(pluginController);

	return 0;
}
