cmake_minimum_required(VERSION 3.10)

project(EgoComponentsPlugins)

add_library(EgoComponentsPlugins INTERFACE)
add_library(Ego::Components::Plugins ALIAS EgoComponentsPlugins)

add_subdirectory(Platform)
add_subdirectory(RenderHardware)

target_link_libraries(
    EgoComponentsPlugins
        INTERFACE
            EgoComponentsPluginsPlatform
            EgoPluginsRenderHardware
)
