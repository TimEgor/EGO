set(EGO_EDITOR_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_editor_config TARGET_NAME)
    set(EGO_EDITOR_ASSETS_DIR "${EGO_EDITOR_DIR}/Assets")
    set(EGO_EDITOR_CONFIG_FILE "Editor.xml")
    set(EGO_EDITOR_CONFIG_TEMPLATE "${EGO_EDITOR_DIR}/Editor.xml.tpl")
    set(EGO_EDITOR_INSTALL_DIR "Editor")
    set(EGO_EDITOR_RUNTIME_ASSETS_DIR "Assets")
    set(EGO_EDITOR_RUNTIME_PLUGIN_DIRECTORY "../Plugins")

    set(TPL_EDITOR_ASSETS_DIR "${EGO_EDITOR_ASSETS_DIR}")
    set(TPL_EDITOR_PLUGIN_DIRECTORY "${CMAKE_BINARY_DIR}/Ego/Sources/Plugins")
    set(TPL_EDITOR_PROFILER_PLUGIN)
    set(TPL_EDITOR_RENDER_PLUGIN)
    set(TPL_EDITOR_GUI_RENDER_PLUGIN)
    set(TPL_EDITOR_GRAPHIC_HARDWARE_PLUGIN)

    if (TARGET EgoWinPIXProfiler)
        set(TPL_EDITOR_PROFILER_PLUGIN "$<TARGET_FILE:EgoWinPIXProfiler>")
        add_dependencies(${TARGET_NAME} EgoWinPIXProfiler)
    endif()

    if (TARGET EgoDefaultRender)
        set(TPL_EDITOR_RENDER_PLUGIN "$<TARGET_FILE:EgoDefaultRender>")
        add_dependencies(${TARGET_NAME} EgoDefaultRender)
    endif()

    if (TARGET EgoDefaultGuiRender)
        set(TPL_EDITOR_GUI_RENDER_PLUGIN "$<TARGET_FILE:EgoDefaultGuiRender>")
        add_dependencies(${TARGET_NAME} EgoDefaultGuiRender)
    endif()

    if (TARGET EgoD3D12GraphicHardware)
        set(TPL_EDITOR_GRAPHIC_HARDWARE_PLUGIN "$<TARGET_FILE:EgoD3D12GraphicHardware>")
        add_dependencies(${TARGET_NAME} EgoD3D12GraphicHardware)
    endif()

    if (TARGET EgoDXCResourceProvider)
        add_dependencies(${TARGET_NAME} EgoDXCResourceProvider)
    endif()

    configure_file(
        "${EGO_EDITOR_CONFIG_TEMPLATE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${EGO_EDITOR_CONFIG_FILE}.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${EGO_EDITOR_CONFIG_FILE}"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/${EGO_EDITOR_CONFIG_FILE}.in"
    )

    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${EGO_EDITOR_CONFIG_FILE}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/${EGO_EDITOR_CONFIG_FILE}"
    )

    if (MSVC)
        set_target_properties(
            ${TARGET_NAME}
            PROPERTIES
                VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        )
    endif()

    install(
        DIRECTORY "${EGO_EDITOR_ASSETS_DIR}"
        DESTINATION "${EGO_EDITOR_INSTALL_DIR}"
    )

    set(EGO_EDITOR_RUNTIME_PROFILER_PLUGIN)
    if (TARGET EgoWinPIXProfiler)
        set(
            EGO_EDITOR_RUNTIME_PROFILER_PLUGIN
            "../Plugins/Profiler/${CMAKE_SHARED_LIBRARY_PREFIX}EgoWinPIXProfiler_\${CMAKE_INSTALL_CONFIG_NAME}_${CMAKE_SYSTEM_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
    endif()

    set(
        EGO_EDITOR_RUNTIME_RENDER_PLUGIN
        "../Plugins/Render/${CMAKE_SHARED_LIBRARY_PREFIX}EgoDefaultRender_\${CMAKE_INSTALL_CONFIG_NAME}_${CMAKE_SYSTEM_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    set(
        EGO_EDITOR_RUNTIME_GUI_RENDER_PLUGIN
        "../Plugins/GuiRender/${CMAKE_SHARED_LIBRARY_PREFIX}EgoDefaultGuiRender_\${CMAKE_INSTALL_CONFIG_NAME}_${CMAKE_SYSTEM_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )

    set(EGO_EDITOR_RUNTIME_GRAPHIC_HARDWARE_PLUGIN)
    if (TARGET EgoD3D12GraphicHardware)
        set(
            EGO_EDITOR_RUNTIME_GRAPHIC_HARDWARE_PLUGIN
            "../Plugins/GraphicHardware/${CMAKE_SHARED_LIBRARY_PREFIX}EgoD3D12GraphicHardware_\${CMAKE_INSTALL_CONFIG_NAME}_${CMAKE_SYSTEM_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
    endif()

    install(CODE
        "file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/${EGO_EDITOR_INSTALL_DIR}\")
        set(TPL_EDITOR_ASSETS_DIR \"${EGO_EDITOR_RUNTIME_ASSETS_DIR}\")
        set(TPL_EDITOR_PLUGIN_DIRECTORY \"${EGO_EDITOR_RUNTIME_PLUGIN_DIRECTORY}\")
        set(TPL_EDITOR_PROFILER_PLUGIN \"${EGO_EDITOR_RUNTIME_PROFILER_PLUGIN}\")
        set(TPL_EDITOR_RENDER_PLUGIN \"${EGO_EDITOR_RUNTIME_RENDER_PLUGIN}\")
        set(TPL_EDITOR_GUI_RENDER_PLUGIN \"${EGO_EDITOR_RUNTIME_GUI_RENDER_PLUGIN}\")
        set(TPL_EDITOR_GRAPHIC_HARDWARE_PLUGIN \"${EGO_EDITOR_RUNTIME_GRAPHIC_HARDWARE_PLUGIN}\")
        configure_file(
            \"${EGO_EDITOR_CONFIG_TEMPLATE}\"
            \"\${CMAKE_INSTALL_PREFIX}/${EGO_EDITOR_INSTALL_DIR}/${EGO_EDITOR_CONFIG_FILE}\"
            @ONLY
        )"
    )
endfunction()
