cmake_minimum_required(VERSION 3.10)

project(EgoDefaultGuiRender)

ego_add_plugin(
    EgoDefaultGuiRender
    ALIAS Ego::Plugins::GuiRender::Default
    CATEGORY GuiRender
)

target_link_libraries(
    EgoDefaultGuiRender
        PUBLIC
            Ego::Core
            Ego::Gui
            Ego::Resource
)

include(${CMAKE_CURRENT_LIST_DIR}/ConfigDefaultGuiRender.cmake)
ego_default_gui_render_config_assets()

ego_register_plugin(
    TARGET EgoDefaultGuiRender
    INSTALL_DIR "Plugins/GuiRender"
    MANIFEST
        "GuiRenderPlugin"
        "Default"
)
