#ifndef __LOGGER_MODULE_HPP__
#define __LOGGER_MODULE_HPP__

//AbstractionLayer
#include "LoggingAbstraction.hpp"
#include "Global.hpp"

class Logger final : public LoggingAbstraction, public Global<Logger> {
    public:
    Logger() : LoggingAbstraction(), Global<Logger>() {}

    ErrorType log(const LogType type, const char *tag, const char *format, ...) override;
    ErrorType logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) override;
};

#endif // __LOGGER_MODULE_HPP__