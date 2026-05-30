cmake_minimum_required(VERSION 3.10)

project(EgoFramework)

ego_add_component(
    EgoFramework
    ALIAS Ego::Framework
)

target_link_libraries(
    EgoFramework
        PUBLIC
            Ego::Engine
)
