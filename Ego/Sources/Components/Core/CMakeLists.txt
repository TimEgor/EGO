cmake_minimum_required(VERSION 3.10)

project(EgoCore)

add_library(EgoCore STATIC)
add_library(Ego::Core ALIAS EgoCore)

set(CORE_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(CORE_SOURCES "${CORE_SOURCES_PATH}")

target_sources(EgoCore PRIVATE ${CORE_SOURCES})
source_group(TREE "${CORE_SOURCES_PATH}/EgoCore" FILES ${CORE_SOURCES})

target_include_directories(EgoCore PUBLIC "${CORE_SOURCES_PATH}")

ego_setup_target_common(
    EgoCore
    "Ego/Components"
)