#ifndef __WATCHDOG_MODULE_HPP__
#define __WATCHDOG_MODULE_HPP__

//AbstractionLayer
#include "WatchdogAbstraction.hpp"

class Watchdog : public WatchdogAbstraction{
    
    public:
    Watchdog() : WatchdogAbstraction(){}
    ~Watchdog() = default;

    ErrorType startPetting() override;
    ErrorType stopPetting() override;
    ErrorType pet() override;
};

#endif // __WATCHDOG_MODULE_HPP__
