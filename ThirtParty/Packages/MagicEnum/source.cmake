ego_third_party_resolve_path("magic_enum/include" MAGIC_ENUM_INCLUDE_DIR)

if (EXISTS "${MAGIC_ENUM_INCLUDE_DIR}/magic_enum/magic_enum.hpp")
    ego_third_party_create_include_target(MagicEnum "${MAGIC_ENUM_INCLUDE_DIR}")
    set(EGO_THIRD_PARTY_FOUND TRUE)
endif()
