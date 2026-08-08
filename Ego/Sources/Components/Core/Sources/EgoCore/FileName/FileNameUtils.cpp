#include "FileNameUtils.h"

#include <filesystem>

ego::FileName ego::file_name_utils::CombinePath(const FileName& _basePath, const FileName& _path)
{
    const std::filesystem::path combinedPath = std::filesystem::path(_basePath.getView()) / _path.getView();

    return FileName(combinedPath.lexically_normal().string());
}

void ego::file_name_utils::GetFileName(const FileName& _path, FileName& _resultName)
{
    const std::filesystem::path tmpPath(_path.getView());
    _resultName = tmpPath.filename().string();
}

ego::FileName ego::file_name_utils::GetFileName(const FileName& _path)
{
    const std::filesystem::path tmpPath(_path.getView());

    return FileName(tmpPath.filename().string());
}

void ego::file_name_utils::GetFileDirPath(const FileName& _path, FileName& _resultPath)
{
    const std::filesystem::path tmpPath(_path.getView());
    _resultPath = tmpPath.parent_path().string();
}

ego::FileName ego::file_name_utils::GetFileDirPath(const FileName& _path)
{
    const std::filesystem::path tmpPath(_path.getView());

    return FileName(tmpPath.parent_path().string());
}

void ego::file_name_utils::GetFileExtension(const FileName& _path, FileName& _resultExtension)
{
    const std::filesystem::path tmpPath(_path.getView());
    _resultExtension = tmpPath.extension().string();
}

ego::FileName ego::file_name_utils::GetFileExtension(const FileName& _path)
{
    const std::filesystem::path tmpPath(_path.getView());

    return FileName(tmpPath.extension().string());
}

void ego::file_name_utils::RemoveExtension(FileName& _name)
{
    std::filesystem::path tmpPath(_name.getView());
    _name = tmpPath.replace_extension().string();
}

ego::FileName ego::file_name_utils::RemoveExtension(const FileName& _name)
{
    std::filesystem::path tmpPath(_name.getView());

    return FileName(tmpPath.replace_extension().string());
}
