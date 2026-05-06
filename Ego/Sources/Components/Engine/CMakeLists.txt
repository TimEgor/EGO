cmake_minimum_required(VERSION 3.10)

project(EgoEngine)

add_library(EgoEngine STATIC)
add_library(Ego::Engine ALIAS EgoEngine)

set(ENGINE_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(ENGINE_SOURCES "${ENGINE_SOURCES_PATH}")

target_sources(EgoEngine PRIVATE ${ENGINE_SOURCES})
source_group(TREE "${ENGINE_SOURCES_PATH}/EgoEngine" FILES ${ENGINE_SOURCES})

target_include_directories(EgoEngine PUBLIC "${ENGINE_SOURCES_PATH}")

target_link_libraries(
    EgoEngine
        PUBLIC
            EgoCore
            EgoMath
            EgoPlugin
)

ego_setup_target_common(
    EgoEngine
    "Ego/Components"
)
