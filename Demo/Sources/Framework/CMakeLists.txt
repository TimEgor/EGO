cmake_minimum_required(VERSION 3.10)

project(EgoDemoFramework)

add_library(EgoDemoFramework STATIC)
add_library(Ego::Demo::Framework ALIAS EgoDemoFramework)

set(CORE_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(CORE_SOURCES "${CORE_SOURCES_PATH}")

target_sources(EgoDemoFramework PRIVATE ${CORE_SOURCES})
source_group(TREE "${CORE_SOURCES_PATH}/EgoDemoFramework" FILES ${CORE_SOURCES})

target_include_directories(EgoDemoFramework PUBLIC "${CORE_SOURCES_PATH}")

target_link_libraries(
    EgoDemoFramework
        PUBLIC
            EgoEngine
)

ego_setup_target_common(
    EgoDemoFramework
    "Demo"
)