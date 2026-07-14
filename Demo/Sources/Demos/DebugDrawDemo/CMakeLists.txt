cmake_minimum_required(VERSION 3.10)

project(EgoDebugDrawDemo)

ego_add_demo_plugin(
    EgoDebugDrawDemo
    ALIAS Ego::Demo::Demos::DebugDrawDemo
)

target_link_libraries(
    EgoDebugDrawDemo
        PRIVATE
            Ego::Core
            Ego::ECS
            Ego::Engine
            Ego::Plugin
)

ego_register_plugin(
    TARGET EgoDebugDrawDemo
    INSTALL_DIR "Demo/Demos/DebugDrawDemo"
    MANIFEST
        "EngineLogicPlugin"
        "DebugDrawDemo"
)

include(${CMAKE_CURRENT_LIST_DIR}/ConfigProject.cmake)
ego_debug_draw_demo_config_project()
