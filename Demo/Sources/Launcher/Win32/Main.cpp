#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/ArgParser/Parser.h"

#include "EgoDemoFramework/DemoController.h"

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
	ego::engine::EngineInitData initData;
	initData.m_nativeInstanceHandle = _hInstance;

	ego::ArgParser argParser;
	argParser.addOptionValue("--platform", initData.m_platformPluginModuleName);
	argParser.addOptionValue("--renderHardware", initData.m_renderHardwarePluginModuleName);

	argParser.parse(__argc, __argv);

	ego::demo::DemoController* demoController = new ego::demo::DemoController();
	ego::demo::DemoControllerCore::GetInstance().init(demoController);

	EGO_CHECK_RETURN_VALUE(demoController->init(initData), 1);

	demoController->run();

	return 0;
}
