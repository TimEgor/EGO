cmake_minimum_required(VERSION 3.10)

project(EgoDefaultRender)

add_library(EgoDefaultRender SHARED)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoDefaultRender PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoDefaultRender" FILES ${PLUGIN_SOURCES})

target_include_directories(EgoDefaultRender PUBLIC "${PLUGIN_SOURCES_PATH}")

target_link_libraries(
    EgoDefaultRender
        PUBLIC
            EgoEngine
)

ego_setup_target_common(
    EgoDefaultRender
    "Plugins/Render"
)

ego_install_target(
    EgoDefaultRender
    "Plugins/Render"
)

ego_setup_plugin_manifest(
    EgoDefaultRender
    "Plugins/Render"
    "RenderPlugin"
    "Default"
)
