cmake_minimum_required(VERSION 3.10)

project(EgoDemoLauncherAndroid)

add_library(EgoDemoLauncherAndroid STATIC)

set(LAUNCHER_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR})

ego_collect_sources(LAUNCHER_SOURCES "${LAUNCHER_SOURCES_PATH}")

target_sources(EgoDemoLauncherAndroid PRIVATE ${LAUNCHER_SOURCES})
source_group(TREE "${LAUNCHER_SOURCES_PATH}" FILES ${LAUNCHER_SOURCES})

set(GLUE_PATH ${ANDROID_NDK}/sources/android/native_app_glue)
target_include_directories(EgoDemoLauncherAndroid PUBLIC ${GLUE_PATH})

target_link_libraries(
    EgoDemoLauncherAndroid
        PUBLIC
            EgoEngine
)

ego_setup_target_common(
    EgoDemoLauncherAndroid
    "Demo/Launcher"
)