ego_third_party_resolve_path("stb" STB_INCLUDE_DIR)

if (EXISTS "${STB_INCLUDE_DIR}/stb_truetype.h")
    ego_third_party_create_include_target(Stb "${STB_INCLUDE_DIR}")
    set(EGO_THIRD_PARTY_FOUND TRUE)
endif()
