cmake_minimum_required(VERSION 3.10)

project(EgoPluginsRender)

add_library(EgoPluginsRender INTERFACE)
add_library(Ego::Plugins::Render ALIAS EgoPluginsRender)

add_subdirectory(Default)

target_link_libraries(
    EgoPluginsRender
        INTERFACE
            Ego::Plugins::Render::Default
)
