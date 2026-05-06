cmake_minimum_required(VERSION 3.10)

project(EgoMath)

add_library(EgoMath STATIC)
add_library(Ego::Math ALIAS EgoMath)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoMath PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoMath" FILES ${PLUGIN_SOURCES})

target_include_directories(EgoMath PUBLIC "${PLUGIN_SOURCES_PATH}")

target_link_libraries(EgoMath PUBLIC EgoCore)

ego_setup_target_common(
    EgoMath
    "Ego/Components"
)