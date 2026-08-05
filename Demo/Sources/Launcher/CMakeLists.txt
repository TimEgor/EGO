cmake_minimum_required(VERSION 3.10)

project(EgoDemoLauncher)

add_library(EgoDemoLauncher STATIC)
add_library(Ego::Demo::Launcher ALIAS EgoDemoLauncher)

ego_setup_target_sources(
    EgoDemoLauncher
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Common/EgoDemoLauncher"
    INCLUDE_SCOPE PUBLIC
)

target_link_libraries(
    EgoDemoLauncher
        PUBLIC
            Ego::Application
            Ego::Runtime
            Ego::Engine
        PRIVATE
            Ego::Core
            Ego::Plugin
)

ego_setup_target_common(
    EgoDemoLauncher
    "Demo/Launcher"
)

if (WIN32)
    add_subdirectory(Win32)
endif()
