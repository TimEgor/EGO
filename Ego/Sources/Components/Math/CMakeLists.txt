cmake_minimum_required(VERSION 3.10)

project(EgoMath)

ego_add_component(
    EgoMath
    ALIAS Ego::Math
)

target_link_libraries(EgoMath PUBLIC Ego::Core)
