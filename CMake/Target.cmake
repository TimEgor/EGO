include_guard()

function(ego_setup_target_common TARGET_NAME FILTER)
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "${TARGET_NAME}_$<CONFIG>_$<PLATFORM_ID>"
        FOLDER "${FILTER}"
    )

    if (MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /Zc:preprocessor
        )
    endif()
endfunction()

function(ego_setup_launch_args TARGET_NAME)
    set(DEBUG_ARGS)

    if (NOT DEFINED EGO_PLATFORM_PLUGIN)
        set(EGO_PLATFORM_PLUGIN)

        if (WIN32)
            set(EGO_PLATFORM_PLUGIN "$<TARGET_FILE:EgoWin32Platform>")
        endif()
    endif()

    if (NOT ${EGO_PLATFORM_PLUGIN} STREQUAL "")
        string(APPEND DEBUG_ARGS " --platform=\"${EGO_PLATFORM_PLUGIN}\"")
    endif()

    if (NOT DEFINED EGO_RENDER_HARDWARE_PLUGIN)
        set(EGO_RENDER_HARDWARE_PLUGIN)

        if (WIN32)
            set(EGO_RENDER_HARDWARE_PLUGIN "$<TARGET_FILE:EgoD3D12RenderHardware>")
        endif()
    endif()

    if (NOT ${EGO_RENDER_HARDWARE_PLUGIN} STREQUAL "")
        string(APPEND DEBUG_ARGS " --renderHardware=\"${EGO_RENDER_HARDWARE_PLUGIN}\"")
    endif()

    if (MSVC)
        set_target_properties(${TARGET_NAME} PROPERTIES
            VS_DEBUGGER_COMMAND_ARGUMENTS "${DEBUG_ARGS}"
        )
    endif()
endfunction()

function(ego_install_target TARGET_NAME DIR)
    install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${DIR})
endfunction()
