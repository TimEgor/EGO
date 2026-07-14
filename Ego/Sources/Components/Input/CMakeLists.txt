cmake_minimum_required(VERSION 3.10)

project(EgoInput)

add_library(EgoInput STATIC)
add_library(Ego::Input ALIAS EgoInput)

ego_setup_target_sources(
    EgoInput
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/EgoInput"
    INCLUDE_SCOPE PUBLIC
)

ego_setup_target_common(
    EgoInput
    "Ego/Components"
)

target_link_libraries(
    EgoInput
        PUBLIC
            Ego::Core
            Ego::Event
)
