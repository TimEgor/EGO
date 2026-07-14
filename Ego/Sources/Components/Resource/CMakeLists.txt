cmake_minimum_required(VERSION 3.10)

project(EgoResource)

add_library(EgoResource STATIC)
add_library(Ego::Resource ALIAS EgoResource)

ego_setup_target_sources(
    EgoResource
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/EgoResource"
    INCLUDE_SCOPE PUBLIC
)

ego_setup_target_common(
    EgoResource
    "Ego/Components"
)

target_link_libraries(
    EgoResource
        PUBLIC
            Ego::Core
            Ego::Job
            Ego::Plugin
)
