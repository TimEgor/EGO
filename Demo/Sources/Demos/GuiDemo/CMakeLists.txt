cmake_minimum_required(VERSION 3.10)

project(EgoGuiDemo)

ego_add_demo_plugin(
    EgoGuiDemo
    ALIAS Ego::Demo::Demos::GuiDemo
)

target_link_libraries(
    EgoGuiDemo
        PRIVATE
            Ego::EngineFramework
            Ego::Gui
)

ego_register_plugin(
    TARGET EgoGuiDemo
    INSTALL_DIR "Demo/Demos/GuiDemo"
    MANIFEST
        "EngineLogicPlugin"
        "GuiDemo"
)

include(${CMAKE_CURRENT_LIST_DIR}/ConfigProject.cmake)
ego_gui_demo_config_project()
