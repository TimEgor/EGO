cmake_minimum_required(VERSION 3.10)

project(EgoPluginsWindowSystem)

add_library(EgoPluginsWindowSystem INTERFACE)
add_library(Ego::Plugins::WindowSystem ALIAS EgoPluginsWindowSystem)

if (WIN32)
    add_subdirectory(Win32)
    target_link_libraries(EgoPluginsWindowSystem INTERFACE Ego::Plugins::WindowSystem::Win32)
endif()
