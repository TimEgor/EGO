include(FindPackageHandleStandardArgs)

set(EnTT_ROOT_DIR "${EGO_THIRD_PARTY_PATH}/entt")

find_path(
    EnTT_INCLUDE_DIR
    NAMES entt/entity/registry.hpp
    PATHS "${EnTT_ROOT_DIR}/src"
    NO_DEFAULT_PATH
)

find_package_handle_standard_args(
    EnTT
    REQUIRED_VARS EnTT_INCLUDE_DIR
)

if (EnTT_FOUND AND NOT TARGET EnTT::EnTT)
    add_library(EnTT::EnTT INTERFACE IMPORTED)
    set_target_properties(
        EnTT::EnTT
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EnTT_INCLUDE_DIR}"
    )
endif()
