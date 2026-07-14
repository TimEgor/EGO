cmake_minimum_required(VERSION 3.10)

project(EgoPlugin)

add_library(EgoPlugin STATIC)
add_library(Ego::Plugin ALIAS EgoPlugin)

ego_setup_target_sources(
    EgoPlugin
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/EgoPlugin"
    INCLUDE_SCOPE PUBLIC
)

ego_setup_target_common(
    EgoPlugin
    "Ego/Components"
)

target_link_libraries(
    EgoPlugin
        PUBLIC
            Ego::Core
)
