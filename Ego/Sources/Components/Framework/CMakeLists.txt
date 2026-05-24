cmake_minimum_required(VERSION 3.10)

project(EgoFramework)

add_library(EgoFramework STATIC)
add_library(Ego::Framework ALIAS EgoFramework)

set(FRAMEWORK_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(FRAMEWORK_SOURCES "${FRAMEWORK_SOURCES_PATH}")

target_sources(EgoFramework PRIVATE ${FRAMEWORK_SOURCES})
source_group(TREE "${FRAMEWORK_SOURCES_PATH}/EgoFramework" FILES ${FRAMEWORK_SOURCES})

target_include_directories(EgoFramework PUBLIC "${FRAMEWORK_SOURCES_PATH}")

target_link_libraries(
    EgoFramework
        PUBLIC
            EgoEngine
)

ego_setup_target_common(
    EgoFramework
    "Ego/Components"
)

