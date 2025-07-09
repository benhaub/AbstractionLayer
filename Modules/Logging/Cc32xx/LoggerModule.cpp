//AbstractionLayer
#include "LoggerModule.hpp"
//C++
#include <cassert>

ErrorType Logger::log(const LogType type, const char *tag, const char *format, ...) {
    assert(nullptr != format);
    assert(nullptr != tag);
    assert(nullptr != display);

    va_list args;
    va_start(args, format);

    switch (type) {
        case LogType::Info:
            Display_vprintf(display, 0, 0, format, args);
            break;
        case LogType::Warning:
            Display_vprintf(display, 0, 0, format, args);
            break;
        case LogType::Error:
            Display_vprintf(display, 0, 0, format, args);
            break;
        case LogType::Debug:
            Display_vprintf(display, 0, 0, format, args);
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}

ErrorType Logger::logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) {
    return ErrorType::NotImplemented;
}