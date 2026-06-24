#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Event/EventController.h"

namespace ego
{
    enum class FileSystemChangeType
    {
        Unknown,
        Added,
        Removed,
        Modified,
        RenamedOldName,
        RenamedNewName
    };

    struct FileSystemChangeEvent final : public Event
    {
        FileName m_directoryPath;
        FileName m_path;
        FileSystemChangeType m_changeType = FileSystemChangeType::Unknown;

        FileSystemChangeEvent() = default;

        FileSystemChangeEvent(const FileName& _directoryPath, const FileName& _path, FileSystemChangeType _changeType)
            : m_directoryPath(_directoryPath),
              m_path(_path),
              m_changeType(_changeType)
        {
        }

        EGO_EVENT(FileSystemChangeEvent, Event);
    };
} // namespace ego
