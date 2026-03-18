#ifndef __LOGGER_MODULE_HPP__
#define __LOGGER_MODULE_HPP__

//AbstractionLayer
#include "LoggingAbstraction.hpp"
#include "PowerResetClockManagementAbstraction.hpp"
#include "Global.hpp"
//TI
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"

class Logger final : public LoggingAbstraction, public Global<Logger> {
    public:
    Logger() : LoggingAbstraction(), Global<Logger>() {
#if defined(PART_TM4C1294NCPDT)
        UARTStdioConfig(static_cast<uint8_t>(PeripheralNumber::One), 115200, PowerResetClockManagementTypes::SystemClockFrequency);
#elif defined(PART_TM4C123GH6PM)
        UARTStdioConfig(static_cast<uint8_t>(PeripheralNumber::One), 115200, SysCtlClockGet());
#endif
    }

    virtual ErrorType log(const LogType type, const char *tag, const char *format, ...) override;
    virtual ErrorType logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) override;
};

#endif // __LOGGER_MODULE_HPP__