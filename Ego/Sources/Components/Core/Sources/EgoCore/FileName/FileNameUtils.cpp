#include "FileNameUtils.h"

#include <filesystem>

void ego::file_name_utils::GetFileName(const FileName& _path, FileName& _resultName)
{
	const std::filesystem::path tmpPath(_path.c_str());
	_resultName = tmpPath.filename().string();
}

ego::FileName ego::file_name_utils::GetFileName(const FileName& _path)
{
	const std::filesystem::path tmpPath(_path.c_str());
	return tmpPath.filename().string();
}

void ego::file_name_utils::GetFileDirPath(const FileName& _path, FileName& _resultPath)
{
	const std::filesystem::path tmpPath(_path.c_str());
	_resultPath = tmpPath.parent_path().string();
}

ego::FileName ego::file_name_utils::GetFileDirPath(const FileName& _path)
{
	const std::filesystem::path tmpPath(_path.c_str());
	return tmpPath.parent_path().string();
}

void ego::file_name_utils::GetFileExtension(const FileName& _path, FileName& _resultExtension)
{
	const std::filesystem::path tmpPath(_path.c_str());
	_resultExtension = tmpPath.extension().string();
}

ego::FileName ego::file_name_utils::GetFileExtension(const FileName& _path)
{
	const std::filesystem::path tmpPath(_path.c_str());
	return tmpPath.extension().string();
}

void ego::file_name_utils::RemoveExtension(FileName& _name)
{
	std::filesystem::path tmpPath(_name.c_str());
	_name = tmpPath.replace_extension().string();
}

ego::FileName ego::file_name_utils::RemoveExtension(const FileName& _name)
{
	std::filesystem::path tmpPath(_name.c_str());
	return tmpPath.replace_extension().string();
}