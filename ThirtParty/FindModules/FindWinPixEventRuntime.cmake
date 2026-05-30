include(FindPackageHandleStandardArgs)

set(WinPixEventRuntime_ROOT "" CACHE PATH "Root directory of WinPixEventRuntime")

set(_WINPIX_ARCH "")
if (CMAKE_GENERATOR_PLATFORM MATCHES "ARM64")
    set(_WINPIX_ARCH "ARM64")
elseif (CMAKE_GENERATOR_PLATFORM MATCHES "x64" OR (DEFINED CMAKE_SIZEOF_VOID_P AND CMAKE_SIZEOF_VOID_P EQUAL 8))
    set(_WINPIX_ARCH "x64")
endif()

set(_WINPIX_ROOTS "")
if (WinPixEventRuntime_ROOT)
    list(APPEND _WINPIX_ROOTS "${WinPixEventRuntime_ROOT}")
endif()
if (DEFINED ENV{WinPixEventRuntime_ROOT})
    list(APPEND _WINPIX_ROOTS "$ENV{WinPixEventRuntime_ROOT}")
endif()
if (DEFINED ENV{WINPIX_EVENT_RUNTIME_ROOT})
    list(APPEND _WINPIX_ROOTS "$ENV{WINPIX_EVENT_RUNTIME_ROOT}")
endif()
list(APPEND _WINPIX_ROOTS
    "${EGO_THIRD_PARTY_PATH}/WinPixEventRuntime"
)

find_path(
    WinPixEventRuntime_INCLUDE_DIR
    NAMES WinPixEventRuntime/pix3.h
    PATHS ${_WINPIX_ROOTS}
    PATH_SUFFIXES Include include
    NO_DEFAULT_PATH
)

if (_WINPIX_ARCH)
    find_library(
        WinPixEventRuntime_LIBRARY
        NAMES WinPixEventRuntime
        PATHS ${_WINPIX_ROOTS}
        PATH_SUFFIXES "Bin/${_WINPIX_ARCH}" "bin/${_WINPIX_ARCH}"
        NO_DEFAULT_PATH
    )

    find_file(
        WinPixEventRuntime_BINARY
        NAMES WinPixEventRuntime.dll
        PATHS ${_WINPIX_ROOTS}
        PATH_SUFFIXES "Bin/${_WINPIX_ARCH}" "bin/${_WINPIX_ARCH}"
        NO_DEFAULT_PATH
    )
endif()

find_package_handle_standard_args(
    WinPixEventRuntime
    REQUIRED_VARS
        WinPixEventRuntime_INCLUDE_DIR
        WinPixEventRuntime_LIBRARY
        WinPixEventRuntime_BINARY
)

if (WinPixEventRuntime_FOUND AND NOT TARGET WinPixEventRuntime)
    add_library(WinPixEventRuntime INTERFACE IMPORTED)
    set_target_properties(
        WinPixEventRuntime
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${WinPixEventRuntime_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${WinPixEventRuntime_LIBRARY}"
    )
endif()
