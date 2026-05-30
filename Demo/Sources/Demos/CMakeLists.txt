cmake_minimum_required(VERSION 3.10)

project(EgoDemoDemos)

add_library(EgoDemoDemos INTERFACE)
add_library(Ego::Demo::Demos ALIAS EgoDemoDemos)

add_subdirectory(TestDemo)

target_link_libraries(
    EgoDemoDemos
        INTERFACE
            Ego::Demo::Demos::TestDemo
)
