cmake_minimum_required(VERSION 3.10)

project(EgoDemoLauncher)

add_library(EgoDemoLauncher INTERFACE)
add_library(Ego::Demo::Launcher ALIAS EgoDemoLauncher)

if (WIN32)
    add_subdirectory(Win32)
    target_link_libraries(EgoDemoLauncher INTERFACE EgoDemoLauncherWin32)
endif()