#include "FileDialog.h"

#if defined(WIN32) || defined(_WIN32)
#include "EgoCore/String/Format.h"

#include <Windows.h>
#include <commdlg.h>

#include <string>

namespace
{
    std::string BuildWin32Filter(
        const ego::OpenFileDialogFilter* _filters,
        const std::size_t _filterCount
    )
    {
        std::string result;

        if (_filters)
        {
            for (std::size_t i = 0; i < _filterCount; ++i)
            {
                const ego::OpenFileDialogFilter& filter = _filters[i];
                if (!filter.m_name || !filter.m_pattern)
                {
                    continue;
                }

                result += filter.m_name;
                result.push_back('\0');
                result += filter.m_pattern;
                result.push_back('\0');
            }
        }

        if (result.empty())
        {
            result += "All Files (*.*)";
            result.push_back('\0');
            result += "*.*";
            result.push_back('\0');
        }

        result.push_back('\0');
        return result;
    }
}
#endif

ego::FileName ego::SelectOpenFile(const OpenFileDialogParams& _params)
{
#if defined(WIN32) || defined(_WIN32)
    char fileName[MAX_PATH] = {};
    const std::string filter = BuildWin32Filter(_params.m_filters, _params.m_filterCount);

    OPENFILENAMEA openFileName = {};
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = _params.m_ownerWindowHandle
        ? static_cast<HWND>(_params.m_ownerWindowHandle)
        : GetActiveWindow();
    openFileName.lpstrFile = fileName;
    openFileName.nMaxFile = MAX_PATH;
    openFileName.lpstrFilter = filter.c_str();
    openFileName.nFilterIndex = 1;
    openFileName.lpstrTitle = _params.m_title;
    openFileName.lpstrDefExt = _params.m_defaultExtension;
    openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetOpenFileNameA(&openFileName))
    {
        const DWORD error = CommDlgExtendedError();
        if (error != 0)
        {
            OutputDebugStringA(StringFormat("Open file dialog error: {}\n", error).c_str());
        }

        return FileName();
    }

    return FileName(fileName);
#else
    (void)_params;
    return FileName();
#endif
}
