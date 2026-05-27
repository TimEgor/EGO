cmake_minimum_required(VERSION 3.10)

project(EgoECS)

add_library(EgoECS STATIC)
add_library(Ego::ECS ALIAS EgoECS)

set(ECS_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(ECS_SOURCES "${ECS_SOURCES_PATH}")

target_sources(EgoECS PRIVATE ${ECS_SOURCES})
source_group(TREE "${ECS_SOURCES_PATH}/EgoECS" FILES ${ECS_SOURCES})

target_include_directories(EgoECS PUBLIC "${ECS_SOURCES_PATH}")

ego_import_third_party_module(EnTT)

target_link_libraries(
    EgoECS
        PUBLIC
            EgoCore
            EnTT::EnTT
)

ego_setup_target_common(
    EgoECS
    "Ego/Components"
)
