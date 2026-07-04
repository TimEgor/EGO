#include "Log.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <string>

#include "EgoCore/Context/DiagnosticContext.h"
#include "EgoCore/UtilsMacros.h"

#if defined(WIN32) || defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#endif

namespace ego::log
{
    class DefaultLogger final : public Logger
    {
    public:
        void write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line) override;

    private:
        static std::string BuildOutputText(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line);

        void writeImpl(LogCategory _category, const std::string& _text) const;

        mutable std::mutex m_lock;
    };

    std::string DefaultLogger::BuildOutputText(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line)
    {
        std::string result;
        result.reserve(_message.size() + 64);

        result += _file ? _file : "Unknown";
        result += '(';
        result += std::to_string(_line);
        result += "): [EGO][";
        result += GetLogCategoryName(_category);
        result += "] ";
        if (!_message.empty())
        {
            result.append(_message.data(), _message.size());
        }

        result += '\n';

        return result;
    }

    void DefaultLogger::write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line)
    {
        const std::string text = BuildOutputText(_category, _message, _file, _line);

        std::lock_guard lock(m_lock);
        writeImpl(_category, text);
    }

#if defined(WIN32) || defined(_WIN32)
    WORD GetConsoleTextColor(LogCategory _category, WORD _defaultAttributes)
    {
        switch (_category)
        {
        case LogCategory::Message:
            return _defaultAttributes;
        case LogCategory::Warning:
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LogCategory::Error:
            return FOREGROUND_RED | FOREGROUND_INTENSITY;
        default:
            return _defaultAttributes;
        }
    }

    void DefaultLogger::writeImpl(LogCategory _category, const std::string& _text) const
    {
        OutputDebugStringA(_text.c_str());

        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console == INVALID_HANDLE_VALUE || console == nullptr)
        {
            return;
        }

        CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
        if (!GetConsoleScreenBufferInfo(console, &consoleInfo))
        {
            return;
        }

        const WORD defaultAttributes = consoleInfo.wAttributes;
        SetConsoleTextAttribute(console, GetConsoleTextColor(_category, defaultAttributes));

        DWORD written = 0;
        const std::size_t maxChunkSize = static_cast<std::size_t>(std::numeric_limits<DWORD>::max());
        const std::size_t writeSize = std::min(_text.size(), maxChunkSize);
        WriteConsoleA(console, _text.data(), static_cast<DWORD>(writeSize), &written, nullptr);

        SetConsoleTextAttribute(console, defaultAttributes);
    }
#else
    void DefaultLogger::writeImpl(LogCategory _category, const std::string& _text) const
    {
        switch (_category)
        {
        case LogCategory::Message:
            std::clog << _text;
            break;
        case LogCategory::Warning:
        case LogCategory::Error:
            std::cerr << _text;
            break;
        default:
            std::clog << _text;
            break;
        }
    }
#endif
} // namespace ego::log

ego::log::LoggerPointer ego::log::CreateDefaultLogger()
{
    return new DefaultLogger();
}

namespace
{
    ego::log::LoggerPointer GetDefaultLogger()
    {
        static const ego::log::LoggerPointer Logger = ego::log::CreateDefaultLogger();
        return Logger;
    }
} // namespace

ego::log::LoggerPointer ego::log::GetLogger()
{
    const context::DiagnosticContextPointer diagnosticContext = context::GetDiagnosticContextPointer();
    if (diagnosticContext && diagnosticContext->getLogger())
    {
        return diagnosticContext->getLogger();
    }

    return nullptr;
}

const char* ego::log::GetLogCategoryName(LogCategory _category)
{
    switch (_category)
    {
    case LogCategory::Message:
        return "MESSAGE";
    case LogCategory::Warning:
        return "WARNING";
    case LogCategory::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

void ego::log::Write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line)
{
#if EGO_ENABLE_LOGS
    const LoggerPointer ideLogger = GetDefaultLogger();
    if (ideLogger)
    {
        ideLogger->write(_category, _message, _file, _line);
    }

    const LoggerPointer logger = GetLogger();
    if (logger && logger.get() != ideLogger.get())
    {
        logger->write(_category, _message, _file, _line);
    }
#else
    (void)_category;
    (void)_message;
    (void)_file;
    (void)_line;
#endif
}
