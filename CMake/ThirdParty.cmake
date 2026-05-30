include_guard()

set(EGO_THIRD_PARTY_PATH "${EGO_ROOT_PATH}/ThirtParty" CACHE PATH "Ego third-party dependency root.")
set(EGO_THIRD_PARTY_PROVIDER "auto" CACHE STRING "Default third-party provider: auto, source, build, or external.")
set(EGO_THIRD_PARTY_BUILD_ROOTS "" CACHE STRING "Semicolon-separated roots with built third-party packages.")
set(EGO_THIRD_PARTY_PROVIDER_VALUES source build external)
set(EGO_THIRD_PARTY_PROVIDER_OPTIONS auto ${EGO_THIRD_PARTY_PROVIDER_VALUES})
set(EGO_THIRD_PARTY_PACKAGES_PATH "${EGO_THIRD_PARTY_PATH}/Packages")
set_property(CACHE EGO_THIRD_PARTY_PROVIDER PROPERTY STRINGS ${EGO_THIRD_PARTY_PROVIDER_OPTIONS})

function(_ego_third_party_key PACKAGE_NAME OUT_VAR)
    string(TOUPPER "${PACKAGE_NAME}" PACKAGE_KEY)
    string(REGEX REPLACE "[^A-Z0-9]" "_" PACKAGE_KEY "${PACKAGE_KEY}")
    set(${OUT_VAR} "${PACKAGE_KEY}" PARENT_SCOPE)
endfunction()

function(ego_third_party_resolve_path INPUT_PATH OUT_VAR)
    if (NOT INPUT_PATH)
        set(${OUT_VAR} "" PARENT_SCOPE)
    elseif (IS_ABSOLUTE "${INPUT_PATH}")
        set(${OUT_VAR} "${INPUT_PATH}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${EGO_THIRD_PARTY_PATH}/${INPUT_PATH}" PARENT_SCOPE)
    endif()
endfunction()

function(ego_get_third_party_build_roots OUT_VAR)
    set(ROOTS)
    foreach(BUILD_ROOT IN LISTS EGO_THIRD_PARTY_BUILD_ROOTS)
        if (NOT BUILD_ROOT)
            continue()
        endif()

        file(TO_CMAKE_PATH "${BUILD_ROOT}" NORMALIZED_BUILD_ROOT)
        if (IS_ABSOLUTE "${NORMALIZED_BUILD_ROOT}")
            list(APPEND ROOTS "${NORMALIZED_BUILD_ROOT}")
        else()
            get_filename_component(
                NORMALIZED_BUILD_ROOT
                "${NORMALIZED_BUILD_ROOT}"
                ABSOLUTE
                BASE_DIR "${EGO_ROOT_PATH}"
            )
            list(APPEND ROOTS "${NORMALIZED_BUILD_ROOT}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES ROOTS)
    set(${OUT_VAR} ${ROOTS} PARENT_SCOPE)
endfunction()

function(ego_third_party_create_link_target PACKAGE_NAME DEPENDENCY_TARGET)
    set(PROJECT_TARGET "EgoThirdParty${PACKAGE_NAME}")
    set(ALIAS_TARGET "Ego::ThirdParty::${PACKAGE_NAME}")

    if (NOT TARGET ${DEPENDENCY_TARGET})
        return()
    endif()

    if (NOT TARGET ${PROJECT_TARGET})
        add_library(${PROJECT_TARGET} INTERFACE)
        target_link_libraries(${PROJECT_TARGET} INTERFACE ${DEPENDENCY_TARGET})
    endif()

    if (NOT TARGET ${ALIAS_TARGET})
        add_library(${ALIAS_TARGET} ALIAS ${PROJECT_TARGET})
    endif()
endfunction()

function(ego_third_party_create_include_target PACKAGE_NAME INCLUDE_DIR)
    set(PROJECT_TARGET "EgoThirdParty${PACKAGE_NAME}")
    set(ALIAS_TARGET "Ego::ThirdParty::${PACKAGE_NAME}")

    if (NOT TARGET ${PROJECT_TARGET})
        add_library(${PROJECT_TARGET} INTERFACE)
        target_include_directories(
            ${PROJECT_TARGET}
                INTERFACE
                    "${INCLUDE_DIR}"
        )
    endif()

    if (NOT TARGET ${ALIAS_TARGET})
        add_library(${ALIAS_TARGET} ALIAS ${PROJECT_TARGET})
    endif()
endfunction()

function(ego_third_party_set_runtime_files PACKAGE_NAME)
    _ego_third_party_key("${PACKAGE_NAME}" PACKAGE_KEY)
    set_property(
        GLOBAL
        PROPERTY "EGO_THIRD_PARTY_${PACKAGE_KEY}_RUNTIME_FILES"
            ${ARGN}
    )
endfunction()

function(ego_third_party_create_binary_target PACKAGE_NAME INCLUDE_DIR LIBRARY_FILE)
    set(PROJECT_TARGET "EgoThirdParty${PACKAGE_NAME}")
    set(ALIAS_TARGET "Ego::ThirdParty::${PACKAGE_NAME}")

    if (NOT TARGET ${PROJECT_TARGET})
        add_library(${PROJECT_TARGET} INTERFACE)
        target_include_directories(
            ${PROJECT_TARGET}
                INTERFACE
                    "${INCLUDE_DIR}"
        )
        target_link_libraries(
            ${PROJECT_TARGET}
                INTERFACE
                    "${LIBRARY_FILE}"
        )
    endif()

    if (NOT TARGET ${ALIAS_TARGET})
        add_library(${ALIAS_TARGET} ALIAS ${PROJECT_TARGET})
    endif()

    if (ARGN)
        ego_third_party_set_runtime_files("${PACKAGE_NAME}" ${ARGN})
    endif()
endfunction()

function(ego_third_party_set_targets_folder PACKAGE_NAME)
    foreach(SOURCE_TARGET IN LISTS ARGN)
        if (NOT TARGET ${SOURCE_TARGET})
            continue()
        endif()

        get_target_property(ALIASED_TARGET ${SOURCE_TARGET} ALIASED_TARGET)
        if (ALIASED_TARGET)
            set(SOURCE_TARGET "${ALIASED_TARGET}")
        endif()

        set_target_properties(
            ${SOURCE_TARGET}
            PROPERTIES
                FOLDER "ThirdParty/${PACKAGE_NAME}"
        )
    endforeach()
endfunction()

function(ego_third_party_apply_imported_config_maps TARGET_NAME)
    if (NOT TARGET ${TARGET_NAME})
        return()
    endif()

    get_target_property(ALIASED_TARGET ${TARGET_NAME} ALIASED_TARGET)
    if (ALIASED_TARGET)
        return()
    endif()

    list(LENGTH ARGN CONFIG_MAP_COUNT)
    math(EXPR CONFIG_MAP_REMAINDER "${CONFIG_MAP_COUNT} % 2")
    if (NOT CONFIG_MAP_REMAINDER EQUAL 0)
        message(FATAL_ERROR "ego_third_party_apply_imported_config_maps expects config/source pairs.")
    endif()

    set(CONFIG_MAP_INDEX 0)
    while(CONFIG_MAP_INDEX LESS CONFIG_MAP_COUNT)
        list(GET ARGN ${CONFIG_MAP_INDEX} DESTINATION_CONFIG)
        math(EXPR CONFIG_MAP_INDEX "${CONFIG_MAP_INDEX} + 1")
        list(GET ARGN ${CONFIG_MAP_INDEX} SOURCE_CONFIG)
        math(EXPR CONFIG_MAP_INDEX "${CONFIG_MAP_INDEX} + 1")

        set_target_properties(
            ${TARGET_NAME}
            PROPERTIES
                MAP_IMPORTED_CONFIG_${DESTINATION_CONFIG} ${SOURCE_CONFIG}
        )
    endwhile()
endfunction()

function(_ego_get_third_party_provider PACKAGE_NAME OUT_VAR)
    _ego_third_party_key("${PACKAGE_NAME}" PACKAGE_KEY)
    set(PACKAGE_PROVIDER_VAR "EGO_THIRD_PARTY_${PACKAGE_KEY}_PROVIDER")

    if (DEFINED ${PACKAGE_PROVIDER_VAR})
        set(PROVIDER "${${PACKAGE_PROVIDER_VAR}}")
    else()
        set(PROVIDER "${EGO_THIRD_PARTY_PROVIDER}")
    endif()

    if (NOT "${PROVIDER}" IN_LIST EGO_THIRD_PARTY_PROVIDER_OPTIONS)
        message(FATAL_ERROR "Unsupported third-party provider '${PROVIDER}' for ${PACKAGE_NAME}.")
    endif()

    set(${OUT_VAR} "${PROVIDER}" PARENT_SCOPE)
endfunction()

function(_ego_get_third_party_provider_order PROVIDER OUT_VAR)
    if (PROVIDER STREQUAL "auto")
        set(PROVIDER_ORDER ${EGO_THIRD_PARTY_PROVIDER_VALUES})
    else()
        set(PROVIDER_ORDER "${PROVIDER}")
    endif()

    set(${OUT_VAR} ${PROVIDER_ORDER} PARENT_SCOPE)
endfunction()

function(_ego_get_third_party_provider_script PACKAGE_NAME PROVIDER OUT_VAR)
    set(PROVIDER_SCRIPT "${EGO_THIRD_PARTY_PACKAGES_PATH}/${PACKAGE_NAME}/${PROVIDER}.cmake")
    if (EXISTS "${PROVIDER_SCRIPT}")
        set(${OUT_VAR} "${PROVIDER_SCRIPT}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "" PARENT_SCOPE)
    endif()
endfunction()

function(_ego_create_third_party_missing_message PACKAGE_NAME PROVIDER OUT_VAR)
    ego_get_third_party_build_roots(BUILD_ROOTS)
    if (BUILD_ROOTS)
        string(REPLACE ";" "\n  " BUILD_ROOTS_TEXT "${BUILD_ROOTS}")
    else()
        set(BUILD_ROOTS_TEXT "<none>")
    endif()

    _ego_third_party_key("${PACKAGE_NAME}" PACKAGE_KEY)

    set(MESSAGE_TEXT
"Third-party dependency '${PACKAGE_NAME}' was not found.
Provider: ${PROVIDER}
Package scripts:
  ${EGO_THIRD_PARTY_PACKAGES_PATH}/${PACKAGE_NAME}
Build roots:
  ${BUILD_ROOTS_TEXT}

Change EGO_THIRD_PARTY_PROVIDER / EGO_THIRD_PARTY_${PACKAGE_KEY}_PROVIDER if needed."
    )

    set(${OUT_VAR} "${MESSAGE_TEXT}" PARENT_SCOPE)
endfunction()

function(ego_require_third_party PACKAGE_NAME)
    set(OPTIONS OPTIONAL)
    set(ONE_VALUE_ARGS PROVIDER RESULT_VAR)
    cmake_parse_arguments(EGO_REQUIRE_THIRD_PARTY "${OPTIONS}" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_REQUIRE_THIRD_PARTY_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_require_third_party received unknown arguments: ${EGO_REQUIRE_THIRD_PARTY_UNPARSED_ARGUMENTS}")
    endif()

    if (TARGET Ego::ThirdParty::${PACKAGE_NAME})
        set(THIRD_PARTY_FOUND TRUE)
    else()
        if (EGO_REQUIRE_THIRD_PARTY_PROVIDER)
            if (NOT "${EGO_REQUIRE_THIRD_PARTY_PROVIDER}" IN_LIST EGO_THIRD_PARTY_PROVIDER_OPTIONS)
                message(FATAL_ERROR "Unsupported third-party provider '${EGO_REQUIRE_THIRD_PARTY_PROVIDER}' for ${PACKAGE_NAME}.")
            endif()
            set(PROVIDER "${EGO_REQUIRE_THIRD_PARTY_PROVIDER}")
        else()
            _ego_get_third_party_provider("${PACKAGE_NAME}" PROVIDER)
        endif()

        set(THIRD_PARTY_FOUND FALSE)
        _ego_get_third_party_provider_order("${PROVIDER}" PROVIDER_ORDER)

        foreach(PROVIDER_ITEM IN LISTS PROVIDER_ORDER)
            _ego_get_third_party_provider_script("${PACKAGE_NAME}" "${PROVIDER_ITEM}" PROVIDER_SCRIPT)
            if (NOT PROVIDER_SCRIPT)
                continue()
            endif()

            set(EGO_THIRD_PARTY_FOUND FALSE)
            include("${PROVIDER_SCRIPT}")
            if (EGO_THIRD_PARTY_FOUND OR TARGET Ego::ThirdParty::${PACKAGE_NAME})
                set(THIRD_PARTY_FOUND TRUE)
                break()
            endif()
        endforeach()
    endif()

    if (NOT DEFINED PROVIDER)
        _ego_get_third_party_provider("${PACKAGE_NAME}" PROVIDER)
    endif()

    if (NOT THIRD_PARTY_FOUND AND NOT EGO_REQUIRE_THIRD_PARTY_OPTIONAL)
        _ego_create_third_party_missing_message("${PACKAGE_NAME}" "${PROVIDER}" MISSING_MESSAGE)
        message(FATAL_ERROR "${MISSING_MESSAGE}")
    endif()

    if (EGO_REQUIRE_THIRD_PARTY_RESULT_VAR)
        set(${EGO_REQUIRE_THIRD_PARTY_RESULT_VAR} ${THIRD_PARTY_FOUND} PARENT_SCOPE)
    endif()
endfunction()

function(ego_copy_third_party_runtime)
    set(ONE_VALUE_ARGS TARGET PACKAGE INSTALL_DIR)
    cmake_parse_arguments(EGO_RUNTIME "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_RUNTIME_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_copy_third_party_runtime received unknown arguments: ${EGO_RUNTIME_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT EGO_RUNTIME_TARGET)
        message(FATAL_ERROR "ego_copy_third_party_runtime requires TARGET.")
    endif()

    if (NOT EGO_RUNTIME_PACKAGE)
        message(FATAL_ERROR "ego_copy_third_party_runtime requires PACKAGE.")
    endif()

    _ego_third_party_key("${EGO_RUNTIME_PACKAGE}" PACKAGE_KEY)
    get_property(RUNTIME_FILES GLOBAL PROPERTY "EGO_THIRD_PARTY_${PACKAGE_KEY}_RUNTIME_FILES")

    foreach(RUNTIME_FILE IN LISTS RUNTIME_FILES)
        add_custom_command(
            TARGET ${EGO_RUNTIME_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RUNTIME_FILE}"
                "$<TARGET_FILE_DIR:${EGO_RUNTIME_TARGET}>"
        )
    endforeach()

    if (EGO_RUNTIME_INSTALL_DIR AND RUNTIME_FILES)
        install(FILES ${RUNTIME_FILES} DESTINATION "${EGO_RUNTIME_INSTALL_DIR}" OPTIONAL)
    endif()
endfunction()
