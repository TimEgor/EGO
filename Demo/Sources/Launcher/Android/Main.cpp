#include <android_native_app_glue.h>

#include "EgoEngine/Engine.h"

int EgoAndroidMain(android_app* _app)
{
	ego::Engine* engine = new ego::Engine();
	ego::EngineCore::GetInstance().setEngine(engine);

	engine->init();

	return 0;
}
