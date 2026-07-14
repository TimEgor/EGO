#include "LogController.h"

#include <iostream>
#include <string>

#include "EgoCore/Assert/Assert.h"
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

        void writeImpl(const std::string& _text) const;

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
        result += LoggerController::GetLogCategoryName(_category);
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
        writeImpl(text);
    }

#if defined(WIN32) || defined(_WIN32)
    void DefaultLogger::writeImpl(const std::string& _text) const
    {
        OutputDebugStringA(_text.c_str());
    }
#else
    void DefaultLogger::writeImpl(const std::string& _text) const
    {
        std::clog << _text;
    }
#endif
} // namespace ego::log

ego::log::LoggerController::~LoggerController()
{
    release();
}

bool ego::log::LoggerController::init()
{
    std::unique_lock lock(m_lock);
    EGO_CHECK_RETURN_FALSE(!m_defaultLogger);

    m_defaultLogger = CreateDefaultLogger();
    return m_defaultLogger.get() != nullptr;
}

void ego::log::LoggerController::release()
{
    LoggerPointer logger;
    {
        std::unique_lock lock(m_lock);
        logger.swap(m_logger);
    }
    logger = nullptr;

    LoggerPointer defaultLogger;
    {
        std::unique_lock lock(m_lock);
        defaultLogger.swap(m_defaultLogger);
    }
}

bool ego::log::LoggerController::setLogger(const LoggerPointer& _logger)
{
    EGO_CHECK_RETURN_FALSE(_logger);

    bool isAlreadySet = false;
    {
        std::unique_lock lock(m_lock);
        isAlreadySet = m_logger && m_logger.get() != _logger.get();
        if (!isAlreadySet)
        {
            m_logger = _logger;
        }
    }

    if (isAlreadySet)
    {
        EGO_ASSERT_FAIL_MESSAGE("Logger has been already set.");
        return false;
    }

    return true;
}

void ego::log::LoggerController::resetLogger(const LoggerPointer& _logger)
{
    LoggerPointer logger;
    bool hasMismatchedLogger = false;
    {
        std::unique_lock lock(m_lock);
        hasMismatchedLogger = _logger && m_logger && m_logger.get() != _logger.get();
        if (!hasMismatchedLogger)
        {
            logger.swap(m_logger);
        }
    }

    if (hasMismatchedLogger)
    {
        EGO_ASSERT_FAIL_MESSAGE("Logger reset request doesn't match active logger.");
    }
}

ego::log::LoggerPointer ego::log::LoggerController::getLogger() const
{
    std::shared_lock lock(m_lock);
    return m_logger;
}

void ego::log::LoggerController::write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line) const
{
    LoggerPointer defaultLogger;
    LoggerPointer logger;
    {
        std::shared_lock lock(m_lock);
        defaultLogger = m_defaultLogger;
        logger = m_logger;
    }

    if (defaultLogger)
    {
        defaultLogger->write(_category, _message, _file, _line);
    }

    if (logger && logger.get() != defaultLogger.get())
    {
        logger->write(_category, _message, _file, _line);
    }
}

const char* ego::log::LoggerController::GetLogCategoryName(LogCategory _category)
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

ego::log::LoggerPointer ego::log::LoggerController::CreateDefaultLogger()
{
    return new DefaultLogger();
}
