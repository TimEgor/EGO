cmake_minimum_required(VERSION 3.10)

project(EgoComponentsPlugins)

add_library(EgoComponentsPlugins INTERFACE)
add_library(Ego::Components::Plugins ALIAS EgoComponentsPlugins)

add_subdirectory(Platform)
add_subdirectory(Profiler)
add_subdirectory(Render)
add_subdirectory(RenderHardware)
add_subdirectory(ResourceProvider)

target_link_libraries(
    EgoComponentsPlugins
        INTERFACE
            Ego::Plugins::Platform
            Ego::Plugins::Profiler
            Ego::Plugins::Render
            Ego::Plugins::RenderHardware
            Ego::Plugins::ResourceProvider
)
