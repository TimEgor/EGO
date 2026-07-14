cmake_minimum_required(VERSION 3.10)

project(EgoJob)

add_library(EgoJob STATIC)
add_library(Ego::Job ALIAS EgoJob)

ego_setup_target_sources(
    EgoJob
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/EgoJob"
    INCLUDE_SCOPE PUBLIC
)

ego_setup_target_common(
    EgoJob
    "Ego/Components"
)

target_link_libraries(
    EgoJob
        PUBLIC
            Ego::Core
)
