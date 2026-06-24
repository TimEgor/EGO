#pragma once

#include "EgoCore/Patterns/Singleton.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/UtilsMacros.h"

#include <cstdint>
#include <mutex>
#include <string_view>

#ifndef EGO_ENABLE_LOGS
    #if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE)
        #define EGO_ENABLE_LOGS 1
    #else
        #define EGO_ENABLE_LOGS 0
    #endif
#endif

namespace ego::log
{
    enum class LogCategory
    {
        Message,
        Warning,
        Error
    };

    class Logger
    {
    public:
        Logger() = default;
        virtual ~Logger() = default;

        virtual void write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line) = 0;
    };

    EGO_POINTER(Logger);

    LoggerPointer CreateDefaultLogger();

    class LogCore final : public Singleton<LogCore>
    {
    public:
        LogCore();

        void setLogger(const LoggerPointer& _logger);
        LoggerPointer getLogger() const;
        LoggerPointer getIdeLogger() const;

    private:
        mutable std::mutex m_lock;
        LoggerPointer m_ideLogger;
        LoggerPointer m_logger;
    };

    LoggerPointer GetLogger();

    const char* GetLogCategoryName(LogCategory _category);

    void Write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line);
} // namespace ego::log

#if EGO_ENABLE_LOGS
    #define EGO_LOG(_CATEGORY, _MESSAGE)                                                                                                                                           \
        do                                                                                                                                                                         \
        {                                                                                                                                                                          \
            ego::log::Write((_CATEGORY), (_MESSAGE), EGO_FILE, EGO_LINE);                                                                                                          \
        } while (false)

    #define EGO_LOG_MESSAGE(_MESSAGE) EGO_LOG(ego::log::LogCategory::Message, (_MESSAGE))
    #define EGO_LOG_WARNING(_MESSAGE) EGO_LOG(ego::log::LogCategory::Warning, (_MESSAGE))
    #define EGO_LOG_ERROR(_MESSAGE) EGO_LOG(ego::log::LogCategory::Error, (_MESSAGE))
#else
    #define EGO_LOG(_CATEGORY, _MESSAGE) ((void)0)
    #define EGO_LOG_MESSAGE(_MESSAGE) ((void)0)
    #define EGO_LOG_WARNING(_MESSAGE) ((void)0)
    #define EGO_LOG_ERROR(_MESSAGE) ((void)0)
#endif
