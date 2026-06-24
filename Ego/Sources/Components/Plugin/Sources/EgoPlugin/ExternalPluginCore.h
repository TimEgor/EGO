#pragma once

#include "EgoCore/PlatformMacros.h"

#define EGO_PLUGIN_CREATE_FUNC_BASE EgoPluginCreate_

#define EGO_PLUGIN_CREATE_FUNC(_TYPE) EGO_CONCAT_DEF(EGO_PLUGIN_CREATE_FUNC_BASE, _TYPE)

#define EGO_PLUGIN_CREATE(_PLUGIN, _TYPE_NAME, _TYPE)                                                                                                                              \
    extern "C" EGO_DYNAMIC_LIB_EXPORT ego::Plugin* __cdecl EGO_PLUGIN_CREATE_FUNC(_TYPE_NAME)(const ego::PluginModulePointer& _module)                                             \
    {                                                                                                                                                                              \
        static_assert(std::is_base_of_v<ego::Plugin, _PLUGIN>);                                                                                                                    \
        return new _PLUGIN(_module, _TYPE::GetPluginType());                                                                                                                       \
    }

#define EGO_MODULE_PLUGIN(_PLUGIN, _TYPE) EGO_PLUGIN_CREATE(_PLUGIN, _TYPE)
