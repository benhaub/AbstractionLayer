#ifndef __LOGGER_MODULE_HPP__
#define __LOGGER_MODULE_HPP__

//AbstractionLayer
#include "LoggingAbstraction.hpp"
#include "Global.hpp"
//TI
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"

class Logger final : public LoggingAbstraction, public Global<Logger> {
    public:
    Logger() : LoggingAbstraction(), Global<Logger>() {
        UARTStdioConfig(static_cast<uint8_t>(PeripheralNumber::One), 115200, SysCtlClockGet());
    }

    virtual ErrorType log(const LogType type, const char *tag, const char *format, ...) override;
    virtual ErrorType logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) override;
};

#endif // __LOGGER_MODULE_HPP__