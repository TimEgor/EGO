include_guard()

function(_ego_create_plugin_manifest_items OUT_VAR)
    list(LENGTH ARGN PLUGIN_ARG_COUNT)
    math(EXPR PLUGIN_ARG_REMAINDER "${PLUGIN_ARG_COUNT} % 2")

    if (PLUGIN_ARG_COUNT LESS 2)
        message(FATAL_ERROR "ego_setup_plugin_manifest expects plugin arguments as Type/Name pairs.")
    endif()

    if (NOT PLUGIN_ARG_REMAINDER EQUAL 0)
        message(FATAL_ERROR "ego_setup_plugin_manifest expects plugin arguments as Type/Name pairs.")
    endif()

    set(PLUGIN_ITEMS "")
    set(PLUGIN_ARG_INDEX 0)
    while(PLUGIN_ARG_INDEX LESS PLUGIN_ARG_COUNT)
        list(GET ARGN ${PLUGIN_ARG_INDEX} PLUGIN_TYPE)
        math(EXPR PLUGIN_ARG_INDEX "${PLUGIN_ARG_INDEX} + 1")

        list(GET ARGN ${PLUGIN_ARG_INDEX} PLUGIN_NAME)
        math(EXPR PLUGIN_ARG_INDEX "${PLUGIN_ARG_INDEX} + 1")

        string(APPEND PLUGIN_ITEMS "    <Plugin Type=\"${PLUGIN_TYPE}\" Name=\"${PLUGIN_NAME}\" />\n")
    endwhile()

    set(${OUT_VAR} "${PLUGIN_ITEMS}" PARENT_SCOPE)
endfunction()

function(_ego_create_plugin_manifest_install_code OUT_VAR MANIFEST_ROOT INSTALL_DIR)
    set(EGO_PLUGIN_MANIFEST_ROOT "${MANIFEST_ROOT}")
    set(EGO_PLUGIN_MANIFEST_INSTALL_DIR "${INSTALL_DIR}")

    set(INSTALL_CODE [=[
set(_ego_plugin_manifest_root "@EGO_PLUGIN_MANIFEST_ROOT@")
set(_ego_plugin_manifest_config "${CMAKE_INSTALL_CONFIG_NAME}")

if("${_ego_plugin_manifest_config}" STREQUAL "")
    file(GLOB _ego_plugin_manifest_files
        "${_ego_plugin_manifest_root}/*.plugin.xml"
        "${_ego_plugin_manifest_root}/*/*.plugin.xml"
    )
else()
    file(GLOB _ego_plugin_manifest_files
        "${_ego_plugin_manifest_root}/${_ego_plugin_manifest_config}/*.plugin.xml"
    )
endif()

foreach(_ego_plugin_manifest_file IN LISTS _ego_plugin_manifest_files)
    file(INSTALL
        DESTINATION "${CMAKE_INSTALL_PREFIX}/@EGO_PLUGIN_MANIFEST_INSTALL_DIR@"
        TYPE FILE
        FILES "${_ego_plugin_manifest_file}"
    )
endforeach()
]=])

    string(CONFIGURE "${INSTALL_CODE}" INSTALL_CODE @ONLY)
    set(${OUT_VAR} "${INSTALL_CODE}" PARENT_SCOPE)
endfunction()

function(ego_setup_plugin_manifest TARGET_NAME DIR)
    _ego_create_plugin_manifest_items(MANIFEST_PLUGIN_ITEMS ${ARGN})

    set(MANIFEST_NAME "$<TARGET_FILE_NAME:${TARGET_NAME}>.plugin.xml")
    set(MANIFEST_ROOT "${CMAKE_BINARY_DIR}/PluginManifests/${TARGET_NAME}")
    set(MANIFEST_BUILD_FILE "${MANIFEST_ROOT}/$<CONFIG>/${MANIFEST_NAME}")

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

    _ego_create_plugin_manifest_install_code(MANIFEST_INSTALL_CODE "${MANIFEST_ROOT}" "${DIR}")
    install(CODE "${MANIFEST_INSTALL_CODE}")
endfunction()
