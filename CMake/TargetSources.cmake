include_guard()

##################################################
#                     GENERAL
##################################################
# # Collect all sources starting from src directory.
# # Traversal is recursive by default, but if a directory
# # contains <dir>/<dir>.cmake, that file controls what is added.

# ego_collect_sources(SRCS src)

##################################################
#                     SUBDIR
##################################################
# # Add files from the current directory using default patterns
# ego_sources_here()

# # Add files from the current directory using custom patterns
# ego_sources_here("*.cpp" "*.h")

# # Add files from a specific subdirectory
# ego_sources_in("impl")

# # Add files from a subdirectory with filter
# ego_sources_in("impl" "*.cpp")

# # Continue recursion only in selected subdirectories
# ego_sources_recurse("a" "b")

# # Continue recursion in all subdirectories
# ego_sources_recurse_all()

# # Add explicit files manually
# ego_sources_add(file1.cpp file2.cpp)
##################################################

# Add explicit list of files to the current collection
function(ego_sources_add)
    list(APPEND EGO_TARGET_SOURCES ${ARGN})
    set(EGO_TARGET_SOURCES "${EGO_TARGET_SOURCES}" PARENT_SCOPE)
endfunction()

function(_ego_default_patterns OUT_VAR)
    set(${OUT_VAR} "*.cpp" "*.c" "*.h" "*.hpp" "*.txt" "*.cmake" PARENT_SCOPE)
endfunction()

# Add files from the current directory using glob patterns.
# If no patterns are provided, default source extensions are used.
function(ego_sources_here)
    set(PATTERNS ${ARGN})
    if(NOT PATTERNS)
        _ego_default_patterns(PATTERNS)
    endif()

    set(GLOBS "")
    foreach(PATTERN IN LISTS PATTERNS)
        list(APPEND GLOBS "${EGO_DIR}/${PATTERN}")
    endforeach()

    file(GLOB FOUND CONFIGURE_DEPENDS ${GLOBS})
    list(APPEND EGO_TARGET_SOURCES ${FOUND})

    set(EGO_TARGET_SOURCES "${EGO_TARGET_SOURCES}" PARENT_SCOPE)
endfunction()

# Add files from a specific subdirectory using glob patterns.
# Directory can be absolute or relative to the current rule directory.
function(ego_sources_in DIR)
    set(PATTERNS ${ARGN})
    if(NOT PATTERNS)
        _ego_default_patterns(PATTERNS)
    endif()

    if(IS_ABSOLUTE "${DIR}")
        set(BASE_DIR "${DIR}")
    else()
        set(BASE_DIR "${EGO_DIR}/${DIR}")
    endif()

    set(GLOBS "")
    foreach(PATTERN IN LISTS PATTERNS)
        list(APPEND GLOBS "${BASE_DIR}/${PATTERN}")
    endforeach()

    file(GLOB FOUND CONFIGURE_DEPENDS ${GLOBS})
    list(APPEND EGO_TARGET_SOURCES ${FOUND})

    set(EGO_TARGET_SOURCES "${EGO_TARGET_SOURCES}" PARENT_SCOPE)
endfunction()

# Continue source collection recursively in the specified subdirectories
function(ego_sources_recurse)
    if (ARGC EQUAL 0)
        message(FATAL_ERROR "ego_sources_recurse expects at least one directory.")
    endif()

    foreach(DIR_ITEM IN LISTS ARGN)
        if(IS_ABSOLUTE "${DIR_ITEM}")
            set(CHILD_DIR "${DIR_ITEM}")
        else()
            set(CHILD_DIR "${EGO_DIR}/${DIR_ITEM}")
        endif()

        if(NOT IS_DIRECTORY "${CHILD_DIR}")
            message(FATAL_ERROR "ego_sources_recurse directory '${DIR_ITEM}' does not exist.")
        endif()

        _ego_collect_dir("${CHILD_DIR}" CHILD_SOURCES)
        list(APPEND EGO_TARGET_SOURCES ${CHILD_SOURCES})
    endforeach()

    set(EGO_TARGET_SOURCES "${EGO_TARGET_SOURCES}" PARENT_SCOPE)
endfunction()

# Continue source collection recursively in all subdirectories
function(ego_sources_recurse_all)
    file(GLOB SUB_DIRS LIST_DIRECTORIES true CONFIGURE_DEPENDS "${EGO_DIR}/*")
    foreach(DIR IN LISTS SUB_DIRS)
        if(IS_DIRECTORY "${DIR}")
            _ego_collect_dir("${DIR}" CHILD)
            list(APPEND EGO_TARGET_SOURCES ${CHILD})
        endif()
    endforeach()

    set(EGO_TARGET_SOURCES "${EGO_TARGET_SOURCES}" PARENT_SCOPE)
endfunction()

function(_ego_collect_dir DIR OUT_SOURCES)
    set(EGO_TARGET_SOURCES "")

    get_filename_component(NAME "${DIR}" NAME)
    set(RULE "${DIR}/${NAME}.cmake")

    if(EXISTS "${RULE}")
        set(EGO_DIR "${DIR}")
        include("${RULE}")

        list(APPEND EGO_TARGET_SOURCES ${RULE})
    else()
        set(EGO_DIR "${DIR}")
        ego_sources_here()

        file(GLOB SUB_DIRS LIST_DIRECTORIES true CONFIGURE_DEPENDS "${DIR}/*")
        foreach(DIR IN LISTS SUB_DIRS)
            if(IS_DIRECTORY "${DIR}")
                _ego_collect_dir("${DIR}" NESTED)
                list(APPEND EGO_TARGET_SOURCES ${NESTED})
            endif()
        endforeach()
    endif()

    set(${OUT_SOURCES} ${EGO_TARGET_SOURCES} PARENT_SCOPE)
endfunction()

# Collect sources starting from the given root directory.
function(ego_collect_sources OUT_SOURCES)
    if(ARGC GREATER 1)
        set(ROOT "${ARGV1}")
        if(NOT IS_ABSOLUTE "${ROOT}")
            set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${ROOT}")
        endif()
    else()
        set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    _ego_collect_dir("${ROOT}" ALL_SOURCES)
    set(${OUT_SOURCES} ${ALL_SOURCES} PARENT_SCOPE)
endfunction()

# Attach a conventional source tree to an already created target.
function(ego_setup_target_sources TARGET_NAME)
    set(ONE_VALUE_ARGS SOURCE_DIR SOURCE_GROUP_ROOT INCLUDE_SCOPE)
    cmake_parse_arguments(EGO_TARGET_SOURCES "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_TARGET_SOURCES_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_setup_target_sources received unknown arguments: ${EGO_TARGET_SOURCES_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT EGO_TARGET_SOURCES_SOURCE_DIR)
        set(EGO_TARGET_SOURCES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Sources")
    endif()

    if (NOT IS_ABSOLUTE "${EGO_TARGET_SOURCES_SOURCE_DIR}")
        set(EGO_TARGET_SOURCES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${EGO_TARGET_SOURCES_SOURCE_DIR}")
    endif()

    if (NOT EGO_TARGET_SOURCES_SOURCE_GROUP_ROOT)
        set(EGO_TARGET_SOURCES_SOURCE_GROUP_ROOT "${EGO_TARGET_SOURCES_SOURCE_DIR}/${TARGET_NAME}")
    endif()

    if (NOT IS_ABSOLUTE "${EGO_TARGET_SOURCES_SOURCE_GROUP_ROOT}")
        set(EGO_TARGET_SOURCES_SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${EGO_TARGET_SOURCES_SOURCE_GROUP_ROOT}")
    endif()

    if (EGO_TARGET_SOURCES_INCLUDE_SCOPE)
        if (NOT EGO_TARGET_SOURCES_INCLUDE_SCOPE MATCHES "^(PRIVATE|PUBLIC|INTERFACE)$")
            message(FATAL_ERROR "ego_setup_target_sources INCLUDE_SCOPE must be PRIVATE, PUBLIC, or INTERFACE.")
        endif()
    endif()

    ego_collect_sources(TARGET_SOURCE_FILES "${EGO_TARGET_SOURCES_SOURCE_DIR}")

    if (TARGET_SOURCE_FILES)
        target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCE_FILES})
        source_group(TREE "${EGO_TARGET_SOURCES_SOURCE_GROUP_ROOT}" FILES ${TARGET_SOURCE_FILES})
    endif()

    if (EGO_TARGET_SOURCES_INCLUDE_SCOPE)
        target_include_directories(
            ${TARGET_NAME}
                ${EGO_TARGET_SOURCES_INCLUDE_SCOPE}
                    "${EGO_TARGET_SOURCES_SOURCE_DIR}"
        )
    endif()
endfunction()
