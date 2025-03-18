//Modules
#include "LoggerModule.hpp"
//ESP
#include "esp_log.h"
//C
#include <assert.h>

ErrorType Logger::log(const LogType type, const char *tag, const char *format, ...) {
    assert(nullptr != format);
    assert(nullptr != tag);

    va_list args;
    va_start(args, format);

    switch (type) {
        case LogType::Info:
            esp_log_writev(ESP_LOG_INFO, tag, format, args);
            break;
        case LogType::Warning:
            esp_log_writev(ESP_LOG_WARN, tag, format, args);
            break;
        case LogType::Error:
            esp_log_writev(ESP_LOG_ERROR, tag, format, args);
            break;
        case LogType::Debug:
            esp_log_writev(ESP_LOG_DEBUG, tag, format, args);
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    va_end(args);

    return ErrorType::Success;
}

ErrorType Logger::logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) {
    switch (logType) {
        case LogType::Info:
            esp_log_buffer_hexdump_internal( tag, buffer, length, ESP_LOG_INFO);
            break;
        case LogType::Warning:
            esp_log_buffer_hexdump_internal( tag, buffer, length, ESP_LOG_WARN);
            break;
        case LogType::Error:
            esp_log_buffer_hexdump_internal( tag, buffer, length, ESP_LOG_ERROR);
            break;
        case LogType::Debug:
            esp_log_buffer_hexdump_internal( tag, buffer, length, ESP_LOG_DEBUG);
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}