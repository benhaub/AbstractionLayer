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

    const auto isAsciiPrintable = [](char c) -> bool {
        return (c >= 32 && c <= 126); // ASCII printable characters range
    };

    for (size_t i = 0, j = 0; i < length; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
        if ((i + 1) % 16 == 0) {
            printf("| ");

            for (; j <= i; j++) {
                if (isAsciiPrintable(buffer[j])) {
                    printf("%c", buffer[j]);
                }
                else {
                    printf(".");
                }
            }
            printf("\n");
        }
    }

    printf("\n");
    return ErrorType::Success;
}