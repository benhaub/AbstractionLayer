#ifndef __LOGGER_MODULE_HPP__
#define __LOGGER_MODULE_HPP__

//AbstractionLayer
#include "LoggingAbstraction.hpp"
#include "Global.hpp"
//TI drivers
#include "ti/display/Display.h"

class Logger final : public LoggingAbstraction, public Global<Logger> {
    public:
    Logger() : LoggingAbstraction(), Global<Logger>() {
        Display_init();

        display = Display_open(Display_Type_UART, NULL);
    }

    virtual ErrorType log(const LogType type, const char *tag, const char *format, ...) override;
    virtual ErrorType logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) override;

    private:
    Display_Handle display = nullptr;
};

#endif // __LOGGER_MODULE_HPP__