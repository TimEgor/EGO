#include <android_native_app_glue.h>

extern int EgoAndroidMain(android_app*);

void android_main(struct android_app* app)
{
    EgoAndroidMain(app);
}