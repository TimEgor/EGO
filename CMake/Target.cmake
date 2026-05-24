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

    if (NOT "${EGO_PLATFORM_PLUGIN}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --platform=\"${EGO_PLATFORM_PLUGIN}\"")
    endif()

    if (NOT DEFINED EGO_RENDER_HARDWARE_PLUGIN)
        set(EGO_RENDER_HARDWARE_PLUGIN)

        if (WIN32)
            set(EGO_RENDER_HARDWARE_PLUGIN "$<TARGET_FILE:EgoD3D12RenderHardware>")
        endif()
    endif()

    if (NOT "${EGO_RENDER_HARDWARE_PLUGIN}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --renderHardware=\"${EGO_RENDER_HARDWARE_PLUGIN}\"")
    endif()

    if (DEFINED EGO_PROJECT_FILE AND NOT "${EGO_PROJECT_FILE}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --project=\"${EGO_PROJECT_FILE}\"")
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

function(ego_setup_plugin_manifest TARGET_NAME DIR PLUGIN_TYPE PLUGIN_NAME)
    set(MANIFEST_NAME "$<TARGET_FILE_NAME:${TARGET_NAME}>.plugin.xml")
    set(MANIFEST_BUILD_FILE "${CMAKE_CURRENT_BINARY_DIR}/PluginManifests/$<CONFIG>/${MANIFEST_NAME}")

    set(MANIFEST_PLUGIN_ITEMS "")
    string(APPEND MANIFEST_PLUGIN_ITEMS "    <Plugin Type=\"${PLUGIN_TYPE}\" Name=\"${PLUGIN_NAME}\" />\n")

    list(LENGTH ARGN MANIFEST_EXTRA_ARG_COUNT)
    math(EXPR MANIFEST_EXTRA_ARG_REMAINDER "${MANIFEST_EXTRA_ARG_COUNT} % 2")
    if (NOT MANIFEST_EXTRA_ARG_REMAINDER EQUAL 0)
        message(FATAL_ERROR "ego_setup_plugin_manifest expects extra plugin arguments as Type/Name pairs.")
    endif()

    set(MANIFEST_EXTRA_ARG_INDEX 0)
    while(MANIFEST_EXTRA_ARG_INDEX LESS MANIFEST_EXTRA_ARG_COUNT)
        list(GET ARGN ${MANIFEST_EXTRA_ARG_INDEX} MANIFEST_EXTRA_PLUGIN_TYPE)
        math(EXPR MANIFEST_EXTRA_ARG_INDEX "${MANIFEST_EXTRA_ARG_INDEX} + 1")
        list(GET ARGN ${MANIFEST_EXTRA_ARG_INDEX} MANIFEST_EXTRA_PLUGIN_NAME)
        math(EXPR MANIFEST_EXTRA_ARG_INDEX "${MANIFEST_EXTRA_ARG_INDEX} + 1")
        string(APPEND MANIFEST_PLUGIN_ITEMS
            "    <Plugin Type=\"${MANIFEST_EXTRA_PLUGIN_TYPE}\" Name=\"${MANIFEST_EXTRA_PLUGIN_NAME}\" />\n"
        )
    endwhile()

    set(MANIFEST_CONTENT [=[
<?xml version="1.0" encoding="utf-8"?>
<PluginModule Platform="$<PLATFORM_ID>" Configuration="$<CONFIG>">
@MANIFEST_PLUGIN_ITEMS@</PluginModule>
]=])
    string(CONFIGURE "${MANIFEST_CONTENT}" MANIFEST_CONTENT @ONLY)

    file(GENERATE
        OUTPUT "${MANIFEST_BUILD_FILE}"
        CONTENT "${MANIFEST_CONTENT}"
    )

    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MANIFEST_BUILD_FILE}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/${MANIFEST_NAME}"
    )

    set(MANIFEST_INSTALL_CODE "
set(_ego_plugin_manifest_config \"\${CMAKE_INSTALL_CONFIG_NAME}\")
if(\"\${_ego_plugin_manifest_config}\" STREQUAL \"\")
    file(GLOB _ego_plugin_manifest_files
        \"${CMAKE_CURRENT_BINARY_DIR}/PluginManifests/*.plugin.xml\"
        \"${CMAKE_CURRENT_BINARY_DIR}/PluginManifests/*/*.plugin.xml\"
    )
else()
    file(GLOB _ego_plugin_manifest_files
        \"${CMAKE_CURRENT_BINARY_DIR}/PluginManifests/\${_ego_plugin_manifest_config}/*.plugin.xml\"
    )
endif()
foreach(_ego_plugin_manifest_file IN LISTS _ego_plugin_manifest_files)
    file(INSTALL
        DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${DIR}\"
        TYPE FILE
        FILES \"\${_ego_plugin_manifest_file}\"
    )
endforeach()
")

    install(CODE "${MANIFEST_INSTALL_CODE}")
endfunction()
