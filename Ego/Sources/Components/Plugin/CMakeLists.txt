cmake_minimum_required(VERSION 3.10)

project(EgoPlugin)

ego_add_component(
    EgoPlugin
    ALIAS Ego::Plugin
)

target_link_libraries(EgoPlugin PUBLIC Ego::Core)
