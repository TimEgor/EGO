cmake_minimum_required(VERSION 3.10)

project(EgoDemoLauncherWin32)

add_executable(EgoDemoLauncherWin32 WIN32)

set(LAUNCHER_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR})

ego_collect_sources(LAUNCHER_SOURCES "${LAUNCHER_SOURCES_PATH}")

target_sources(EgoDemoLauncherWin32 PRIVATE ${LAUNCHER_SOURCES})
source_group(TREE "${LAUNCHER_SOURCES_PATH}" FILES ${LAUNCHER_SOURCES})

target_link_libraries(EgoDemoLauncherWin32 PRIVATE EgoFramework)

if (TARGET EgoTestDemo)
    add_dependencies(EgoDemoLauncherWin32 EgoTestDemo)
endif()

ego_setup_target_common(
    EgoDemoLauncherWin32
    "Demo/Launcher"
)

ego_install_target(
    EgoDemoLauncherWin32
    "Demo/Launcher"
)

ego_setup_launch_args(
    EgoDemoLauncherWin32
)
