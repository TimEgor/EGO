#include <android_native_app_glue.h>

#include "EgoEngine/Engine.h"

int EgoAndroidMain(android_app* _app)
{
	ego::engine::EnginePointer engine(new ego::engine::Engine());
	ego::engine::EngineCore::GetInstance().init(engine);

	ego::engine::EngineInitData initData;
	initData.m_nativeInstanceHandle = _app;
	engine->init(initData);

	return 0;
}
