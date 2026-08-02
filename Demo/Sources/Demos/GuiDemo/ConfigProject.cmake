set(EGO_GUI_DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_gui_demo_config_project)
    set(TPL_GUI_DEMO_PLUGINS_PATH "${CMAKE_INSTALL_PREFIX}/Plugins")
    set(TPL_GUI_DEMO_GAME_PLUGINS_PATH "$<TARGET_FILE_DIR:EgoGuiDemo>")

    configure_file(
        "${EGO_GUI_DEMO_DIR}/GuiDemoProject.egoproj.tpl"
        "${CMAKE_CURRENT_BINARY_DIR}/GuiDemoProject.egoproj.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/GuiDemoProject.egoproj"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/GuiDemoProject.egoproj.in"
    )

    add_custom_command(
        TARGET EgoGuiDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/GuiDemoProject.egoproj"
            $<TARGET_FILE_DIR:EgoGuiDemo>/GuiDemoProject.egoproj
    )

    if (POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    install(CODE
        "set(TPL_GUI_DEMO_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Plugins\")
        set(TPL_GUI_DEMO_GAME_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/GuiDemo\")
        configure_file(
            \"${EGO_GUI_DEMO_DIR}/GuiDemoProject.egoproj.tpl\"
            \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/GuiDemo/GuiDemoProject.egoproj\"
            @ONLY
        )"
    )
endfunction()
