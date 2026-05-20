cmake_minimum_required(VERSION 3.10)

project(EgoTestDemo)

add_library(EgoTestDemo SHARED)

set(DEMO_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(DEMO_SOURCES "${DEMO_SOURCES_PATH}")

target_sources(EgoTestDemo PRIVATE ${DEMO_SOURCES})
source_group(TREE "${DEMO_SOURCES_PATH}/EgoTestDemo" FILES ${DEMO_SOURCES})

target_include_directories(EgoTestDemo PRIVATE "${DEMO_SOURCES_PATH}")

target_link_libraries(
    EgoTestDemo
        PRIVATE
            EgoDemoFramework
)

ego_setup_target_common(
    EgoTestDemo
    "Demo/Demos"
)

ego_install_target(
    EgoTestDemo
    "Demo/Demos"
)

include(${CMAKE_CURRENT_LIST_DIR}/InstallAssetsAndConfig.cmake)
ego_test_demo_install_assets_and_config()
