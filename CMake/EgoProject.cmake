include_guard()

get_filename_component(EGO_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

include("${EGO_ROOT_PATH}/CMake/Common.cmake")
include("${EGO_ROOT_PATH}/CMake/Configurations.cmake")
include("${EGO_ROOT_PATH}/CMake/TargetSources.cmake")
include("${EGO_ROOT_PATH}/CMake/Target.cmake")
include("${EGO_ROOT_PATH}/CMake/ThirdParty.cmake")

macro(ego_setup_project)
    ego_setup_global_properties()
endmacro()
