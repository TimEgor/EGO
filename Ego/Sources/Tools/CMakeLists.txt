cmake_minimum_required(VERSION 3.10)

project(EgoTools)

add_library(EgoTools INTERFACE)
add_library(Ego::Tools ALIAS EgoTools)

add_subdirectory(Application)

target_link_libraries(
    EgoTools
        INTERFACE
            Ego::Application
)
