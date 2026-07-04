cmake_minimum_required(VERSION 3.10)

project(EgoRuntime)

ego_add_component(
    EgoRuntime
    ALIAS Ego::Runtime
)

target_link_libraries(
    EgoRuntime
        PUBLIC
            Ego::Core
)
