cmake_minimum_required(VERSION 3.10)

project(EgoPlugin)

add_library(EgoPlugin STATIC)
add_library(Ego::Plugin ALIAS EgoPlugin)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoPlugin PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoPlugin" FILES ${PLUGIN_SOURCES})

target_include_directories(EgoPlugin PUBLIC "${PLUGIN_SOURCES_PATH}")

target_link_libraries(EgoPlugin PUBLIC EgoCore)

ego_setup_target_common(
    EgoPlugin
    "Ego/Components"
)