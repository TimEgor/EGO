cmake_minimum_required(VERSION 3.10)

project(EgoGuiRender)

ego_add_component(
    EgoGuiRender
    ALIAS Ego::GuiRender
)

target_link_libraries(
    EgoGuiRender
        PUBLIC
            Ego::Core
            Ego::GraphicHardware
            Ego::Gui
            Ego::Plugin
)
