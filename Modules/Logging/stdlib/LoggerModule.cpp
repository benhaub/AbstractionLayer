//Moudles
#include "LoggerModule.hpp"
//C++
#include <cstdarg>
#include <cstdio>

ErrorType Logger::log(const LogType type, const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);

    switch (type) {
        case LogType::Info:
            vprintf(format, args);
            break;
        case LogType::Warning:
            vprintf(format, args);
            break;
        case LogType::Error:
            vprintf(format, args);
            break;
        case LogType::Debug:
            vprintf(format, args);
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