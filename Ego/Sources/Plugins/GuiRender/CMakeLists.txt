cmake_minimum_required(VERSION 3.10)

project(EgoPluginsGuiRender)

add_library(EgoPluginsGuiRender INTERFACE)
add_library(Ego::Plugins::GuiRender ALIAS EgoPluginsGuiRender)

add_subdirectory(Default)

target_link_libraries(
    EgoPluginsGuiRender
        INTERFACE
            Ego::Plugins::GuiRender::Default
)
