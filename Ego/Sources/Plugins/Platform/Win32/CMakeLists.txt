cmake_minimum_required(VERSION 3.10)

project(EgoWin32Platform)

add_library(EgoWin32Platform SHARED)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoWin32Platform PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoWin32Platform" FILES ${PLUGIN_SOURCES})

target_include_directories(EgoWin32Platform PRIVATE "${PLUGIN_SOURCES_PATH}")

target_link_libraries(EgoWin32Platform
    PRIVATE
        EgoEngine
)

ego_setup_target_common(
    EgoWin32Platform
    "Plugins/Platform"
)

ego_install_target(
    EgoWin32Platform
    "Plugins/Platform"
)

ego_setup_plugin_manifest(
    EgoWin32Platform
    "Plugins/Platform"
    "PlatformPlugin"
    "Win32"
)
