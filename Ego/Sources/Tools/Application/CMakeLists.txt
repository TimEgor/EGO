cmake_minimum_required(VERSION 3.10)

project(EgoApplication)

ego_add_tool(
    EgoApplication
    ALIAS Ego::Application
)

target_link_libraries(
    EgoApplication
        PUBLIC
            Ego::Engine
)
