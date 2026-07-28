ego_third_party_resolve_path("imgui" EGO_DEAR_IMGUI_SOURCE_DIR)

set(EGO_DEAR_IMGUI_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include")
set(
    EGO_DEAR_IMGUI_SOURCES
    "${EGO_DEAR_IMGUI_SOURCE_DIR}/imgui.cpp"
    "${EGO_DEAR_IMGUI_SOURCE_DIR}/imgui_demo.cpp"
    "${EGO_DEAR_IMGUI_SOURCE_DIR}/imgui_draw.cpp"
    "${EGO_DEAR_IMGUI_SOURCE_DIR}/imgui_tables.cpp"
    "${EGO_DEAR_IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
    "${EGO_DEAR_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
)

set(EGO_DEAR_IMGUI_SOURCE_FOUND TRUE)
foreach(EGO_DEAR_IMGUI_SOURCE IN LISTS EGO_DEAR_IMGUI_SOURCES)
    if (NOT EXISTS "${EGO_DEAR_IMGUI_SOURCE}")
        set(EGO_DEAR_IMGUI_SOURCE_FOUND FALSE)
        break()
    endif()
endforeach()

if (EGO_DEAR_IMGUI_SOURCE_FOUND)
    add_library(
        EgoDearImGui
        STATIC
            ${EGO_DEAR_IMGUI_SOURCES}
            "${EGO_DEAR_IMGUI_INCLUDE_DIR}/EgoDearImGui/ImGuiConfig.h"
    )

    set_target_properties(
        EgoDearImGui
        PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(
        EgoDearImGui
            SYSTEM
                PUBLIC
                    "${EGO_DEAR_IMGUI_SOURCE_DIR}"
    )
    target_include_directories(
        EgoDearImGui
            PUBLIC
                "${EGO_DEAR_IMGUI_INCLUDE_DIR}"
    )

    target_compile_definitions(
        EgoDearImGui
            PUBLIC
                IMGUI_USER_CONFIG=\"EgoDearImGui/ImGuiConfig.h\"
    )

    ego_third_party_create_link_target(DearImGui EgoDearImGui)
    ego_third_party_set_targets_folder(
        DearImGui
        EgoDearImGui
        EgoThirdPartyDearImGui
    )

    set(EGO_THIRD_PARTY_FOUND TRUE)
endif()
