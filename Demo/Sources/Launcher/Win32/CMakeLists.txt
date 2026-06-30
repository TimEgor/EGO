cmake_minimum_required(VERSION 3.10)

project(EgoDemoLauncherWin32)

add_executable(EgoDemoLauncherWin32 WIN32)

ego_setup_target_sources(
    EgoDemoLauncherWin32
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    EgoDemoLauncherWin32
        PRIVATE
            Ego::Application
            Ego::Framework
)

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
