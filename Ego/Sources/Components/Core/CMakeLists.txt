cmake_minimum_required(VERSION 3.10)

project(EgoCore)

ego_add_component(
    EgoCore
    ALIAS Ego::Core
)

ego_require_third_party(PugiXML)

target_link_libraries(
    EgoCore
        PUBLIC
            Ego::ThirdParty::PugiXML
)

if(WIN32)
    target_link_libraries(EgoCore PUBLIC Comdlg32)
endif()
