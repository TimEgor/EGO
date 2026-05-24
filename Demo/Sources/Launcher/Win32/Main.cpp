#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoFramework/Framework.h"
#include "EgoFramework/ProjectReader.h"

#include <string>

namespace
{
	bool LoadProject(const ego::FileName& _fileName, ego::framework::ProjectPointer& _project)
	{
		_project = nullptr;
		EGO_CHECK_RETURN_FALSE(_fileName);

		_project = new ego::framework::Project();
		EGO_CHECK_RETURN_FALSE(_project);
		EGO_CHECK_RETURN_FALSE(ego::framework::ProjectReader::ReadFromFile(_fileName, *_project));

		return true;
	}
}

int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nShowCmd)
{
	ego::framework::Framework::InitData initData;
	initData.m_engineInitData.m_nativeInstanceHandle = _hInstance;
	std::string platformPluginModuleName;
	std::string renderHardwarePluginModuleName;
	std::string projectFilePath;

	ego::ArgParser argParser;
	argParser.addOptionValue("--platform", platformPluginModuleName);
	argParser.addOptionValue("--renderHardware", renderHardwarePluginModuleName);
	argParser.addOptionValue("--project", projectFilePath);

	argParser.parse(__argc, __argv);

	initData.m_engineInitData.m_platformPluginModuleName = platformPluginModuleName;
	initData.m_engineInitData.m_renderHardwarePluginModuleName = renderHardwarePluginModuleName;

	const ego::FileName projectFileName(projectFilePath);
	if (projectFileName)
	{
		EGO_CHECK_RETURN_VALUE(LoadProject(projectFileName, initData.m_project), 2);
	}

	ego::framework::FrameworkPointer framework = new ego::framework::Framework();
	EGO_CHECK_RETURN_VALUE(ego::framework::FrameworkCore::GetInstance().init(framework), 1);
	EGO_CHECK_RETURN_VALUE(framework->init(initData), 10);

	framework->run();

	ego::framework::FrameworkCore::GetInstance().release();
	EGO_SAFE_RESET_POINTER_WITH_RELEASING(framework);

	return 0;
}
