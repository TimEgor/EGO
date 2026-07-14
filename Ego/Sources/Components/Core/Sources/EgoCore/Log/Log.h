#pragma once

#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"
#include "LogController.h"

#ifndef EGO_ENABLE_LOGS
    #if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE)
        #define EGO_ENABLE_LOGS 1
    #else
        #define EGO_ENABLE_LOGS 0
    #endif
#endif

#if EGO_ENABLE_LOGS
    #define EGO_LOG(_CATEGORY, _MESSAGE)                                                                                                                                           \
        do                                                                                                                                                                         \
        {                                                                                                                                                                          \
            const ego::log::LoggerControllerPointer loggerController = ego::GetLoggerController();                                                                                 \
            if (loggerController)                                                                                                                                                  \
            {                                                                                                                                                                      \
                loggerController->write((_CATEGORY), (_MESSAGE), EGO_FILE, EGO_LINE);                                                                                              \
            }                                                                                                                                                                      \
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
