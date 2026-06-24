#pragma once

#include "EgoCore/FileName/FileName.h"

#include <cstddef>

namespace ego
{
    struct OpenFileDialogFilter final
    {
        const char* m_name = nullptr;
        const char* m_pattern = nullptr;
    };

    struct OpenFileDialogParams final
    {
        const char* m_title = nullptr;
        const char* m_defaultExtension = nullptr;
        const OpenFileDialogFilter* m_filters = nullptr;
        std::size_t m_filterCount = 0;
        void* m_ownerWindowHandle = nullptr;
    };

    FileName SelectOpenFile(const OpenFileDialogParams& _params);
} // namespace ego
