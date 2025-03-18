#include "LoggerModule.hpp"

ErrorType Logger::log(const LogType type, const char *tag, const char *format, ...) {

    switch (type) {
        case LogType::Info:
            break;
        case LogType::Warning:
            break;
        case LogType::Error:
            break;
        case LogType::Debug:
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}

ErrorType Logger::logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) {
    return ErrorType::NotImplemented;
}