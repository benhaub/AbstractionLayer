//AbstractionLayer
#include "LoggerModule.hpp"

ErrorType Logger::log(const LogType type, const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);

    switch (type) {
        case LogType::Info:
        case LogType::Warning:
        case LogType::Error:
        case LogType::Debug:
            UARTvprintf(format, args);
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    va_end(args);

    return ErrorType::Success;
}

ErrorType Logger::logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) {
    return ErrorType::NotImplemented;
}