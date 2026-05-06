cmake_minimum_required(VERSION 3.10)

project(EgoAndroidPlatform)

add_library(EgoAndroidPlatform SHARED)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoAndroidPlatform PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoAndroidPlatform" FILES ${PLUGIN_SOURCES})

target_link_libraries(EgoAndroidPlatform
    PRIVATE
        EgoEngine
)

ego_setup_target_common(
    EgoAndroidPlatform
    "Plugins/Platform"
)