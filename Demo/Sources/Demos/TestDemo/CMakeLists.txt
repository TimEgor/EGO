cmake_minimum_required(VERSION 3.10)

project(EgoTestDemo)

ego_add_demo_plugin(
    EgoTestDemo
    ALIAS Ego::Demo::Demos::TestDemo
)

target_link_libraries(
    EgoTestDemo
        PRIVATE
            Ego::EngineFramework
)

ego_register_plugin(
    TARGET EgoTestDemo
    INSTALL_DIR "Demo/Demos/TestDemo"
    MANIFEST
        "EngineLogicPlugin"
        "TestDemo"
)

include(${CMAKE_CURRENT_LIST_DIR}/ConfigProject.cmake)
ego_test_demo_config_project()
