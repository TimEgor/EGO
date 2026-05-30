cmake_minimum_required(VERSION 3.10)

project(EgoECS)

ego_add_component(
    EgoECS
    ALIAS Ego::ECS
)

ego_require_third_party(EnTT)

target_link_libraries(
    EgoECS
        PUBLIC
            Ego::Core
            Ego::ThirdParty::EnTT
)
