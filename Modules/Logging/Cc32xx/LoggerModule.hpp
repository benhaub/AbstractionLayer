#ifndef __LOGGER_MODULE_HPP__
#define __LOGGER_MODULE_HPP__

//Abstractions
#include "LoggingAbstraction.hpp"
//Utilities
#include "Global.hpp"
#include "EventQueue.hpp"
//TI drivers
#include <ti/display/Display.h>

class Logger : public LoggingAbstraction, public Global<Logger>, public EventQueue {
    public:
    Logger() : LoggingAbstraction() {
        Display_init();

        display = Display_open(Display_Type_UART, NULL);
    }
    virtual ~Logger() = default;

    virtual ErrorType log(const LogType type, const char *tag, const char *format, ...) override;
    virtual ErrorType logBuffer(const LogType logType, const char *tag, const char *buffer, Bytes length) override;

    private:
    Display_Handle display = nullptr;
};

#endif // __LOGGER_MODULE_HPP__