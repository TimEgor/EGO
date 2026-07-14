#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <string_view>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

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

    class LoggerController final : public NonCopyable
    {
    public:
        LoggerController() = default;
        ~LoggerController() override;

        bool init();
        void release();

        bool setLogger(const LoggerPointer& _logger);
        void resetLogger(const LoggerPointer& _logger = nullptr);

        LoggerPointer getLogger() const;

        void write(LogCategory _category, std::string_view _message, const char* _file, uint32_t _line) const;

        static const char* GetLogCategoryName(LogCategory _category);

    private:
        static LoggerPointer CreateDefaultLogger();

        mutable std::shared_mutex m_lock;
        LoggerPointer m_defaultLogger = nullptr;
        LoggerPointer m_logger = nullptr;
    };

    EGO_POINTER(LoggerController);
} // namespace ego::log
