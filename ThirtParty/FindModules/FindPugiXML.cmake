set(PugiXML_DIR ${EGO_THIRD_PARTY_PATH}/Install/${CMAKE_GENERATOR_PLATFORM}/PugiXML/lib/cmake/pugixml)

if (EXISTS "${PugiXML_DIR}")
    find_package(PugiXML REQUIRED CONFIG)

    if (TARGET pugixml::static)
        set_target_properties(pugixml::static PROPERTIES MAP_IMPORTED_CONFIG_RETAIL RELEASE)
        set_target_properties(pugixml::static PROPERTIES MAP_IMPORTED_CONFIG_PROFILE RELWITHDEBINFO)
        set_target_properties(pugixml::static PROPERTIES MAP_IMPORTED_CONFIG_RELEASE RELWITHDEBINFO)
    endif()
endif()
