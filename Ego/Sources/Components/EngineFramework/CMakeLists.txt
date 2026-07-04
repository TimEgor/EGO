cmake_minimum_required(VERSION 3.10)

project(EgoEngineFramework)

ego_add_component(
    EgoEngineFramework
    ALIAS Ego::EngineFramework
)

target_link_libraries(
    EgoEngineFramework
        PUBLIC
            Ego::Core
            Ego::Engine
            Ego::Runtime
)
