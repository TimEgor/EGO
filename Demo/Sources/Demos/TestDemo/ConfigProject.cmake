set(EGO_TEST_DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_test_demo_config_project)
    set(EGO_TEST_DEMO_ASSETS_DIR "${EGO_TEST_DEMO_DIR}/Assets")
    set(EGO_TEST_DEMO_PROJECT_FILE "TestDemoProject.xml")
    set(EGO_TEST_DEMO_PROJECT_TEMPLATE "${EGO_TEST_DEMO_DIR}/TestDemoProject.xml.tpl")
    set(EGO_TEST_DEMO_INSTALL_DIR "Demo/Demos/TestDemo")
    set(EGO_TEST_DEMO_RUNTIME_ASSETS_DIR "Assets")

    set(TPL_TEST_DEMO_ASSETS_PATH "${EGO_TEST_DEMO_ASSETS_DIR}")
    set(TPL_TEST_DEMO_PLUGINS_PATH "${CMAKE_INSTALL_PREFIX}/Plugins")
    set(TPL_TEST_DEMO_GAME_PLUGINS_PATH "$<TARGET_FILE_DIR:EgoTestDemo>")

    configure_file(
        "${EGO_TEST_DEMO_PROJECT_TEMPLATE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${EGO_TEST_DEMO_PROJECT_FILE}.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${EGO_TEST_DEMO_PROJECT_FILE}"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/${EGO_TEST_DEMO_PROJECT_FILE}.in"
    )

    add_custom_command(
        TARGET EgoTestDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory
            "$<TARGET_FILE_DIR:EgoTestDemo>/${EGO_TEST_DEMO_RUNTIME_ASSETS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${EGO_TEST_DEMO_PROJECT_FILE}"
            "$<TARGET_FILE_DIR:EgoTestDemo>/${EGO_TEST_DEMO_PROJECT_FILE}"
    )

    if (POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    install(CODE
        "set(TPL_TEST_DEMO_ASSETS_PATH \"\${CMAKE_INSTALL_PREFIX}/${EGO_TEST_DEMO_INSTALL_DIR}/${EGO_TEST_DEMO_RUNTIME_ASSETS_DIR}\")
        set(TPL_TEST_DEMO_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Plugins\")
        set(TPL_TEST_DEMO_GAME_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/${EGO_TEST_DEMO_INSTALL_DIR}\")
        configure_file(
            \"${EGO_TEST_DEMO_PROJECT_TEMPLATE}\"
            \"\${CMAKE_INSTALL_PREFIX}/${EGO_TEST_DEMO_INSTALL_DIR}/${EGO_TEST_DEMO_PROJECT_FILE}\"
            @ONLY
        )"
    )

    install(
        DIRECTORY "${EGO_TEST_DEMO_ASSETS_DIR}"
        DESTINATION "${EGO_TEST_DEMO_INSTALL_DIR}"
        OPTIONAL
    )
endfunction()
