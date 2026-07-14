cmake_minimum_required(VERSION 3.10)

project(EgoEvent)

add_library(EgoEvent STATIC)
add_library(Ego::Event ALIAS EgoEvent)

ego_setup_target_sources(
    EgoEvent
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/EgoEvent"
    INCLUDE_SCOPE PUBLIC
)

ego_setup_target_common(
    EgoEvent
    "Ego/Components"
)

target_link_libraries(
    EgoEvent
        PUBLIC
            Ego::Core
)
