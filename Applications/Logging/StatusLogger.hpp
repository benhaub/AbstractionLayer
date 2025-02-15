/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   StatusLogger.hpp
* @details \b Synopsis: \n Periodic logging of all modules that offer a status
* @ingroup Applications
*******************************************************************************/
#ifndef __STATUS_LOGGER_HPP__
#define __STATUS_LOGGER_HPP__

//AbstractionLayer
#include "Log.hpp"
#include "OperatingSystemModule.hpp"

class HttpServerAbstraction;
class IpClientAbstraction;
class IpServerAbstraction;
class CellularAbstraction;
class NetworkAbstraction;
class WifiAbstraction;
class FileSystemAbstraction;
class StorageAbstraction;

class StatusLogger {

    public:
    StatusLogger(Seconds interval) {
        if (ErrorType::Success == OperatingSystem::Instance().createTimer(_logTimer, interval*1000, true, std::bind(&StatusLogger::printLog, this))) {
            if (ErrorType::Success != OperatingSystem::Instance().startTimer(_logTimer, 0)) {
                PLT_LOGW(TAG, "Failed to start timer for status logging");
            }

            _interval = interval;
        }
    }
    ~StatusLogger() {
        OperatingSystem::Instance().stopTimer(_logTimer, 0);
        OperatingSystem::Instance().deleteTimer(_logTimer);
    }

    /// @brief Tag for logging
    static constexpr char TAG[] = "StatusLogger";

    /// @brief Get the logging interval as a mutable reference
    Seconds &loggingInterval() { return _interval; }
    /// @brief Get the logging interval as a constant reference
    const Seconds &loggingInterval() const { return _interval; }

    ErrorType toggleLoggingFor(IpClientAbstraction *ipClient, bool toggleOn);
    ErrorType toggleLoggingFor(IpServerAbstraction *ipServer, bool toggleOn);
    ErrorType toggleLoggingFor(NetworkAbstraction *network, bool toggleOn);
    ErrorType toggleLoggingFor(OperatingSystemAbstraction *operatingSystem, bool toggleOn);
    ErrorType toggleLoggingFor(FileSystemAbstraction *filesystem, bool toggleOn);
    ErrorType toggleLoggingFor(StorageAbstraction *storage, bool toggleOn);

    private:
    Seconds _interval = 60;
    Id _logTimer;

    IpClientAbstraction *_ipClient = nullptr;
    IpServerAbstraction *_ipServer = nullptr;
    CellularAbstraction *_cellular = nullptr;
    NetworkAbstraction *_network = nullptr;
    WifiAbstraction *_wifi = nullptr;
    OperatingSystemAbstraction *_operatingSystem = nullptr;
    FileSystemAbstraction *_filesystem = nullptr;
    StorageAbstraction *_storage = nullptr;

    void printLog(void);
};
#endif /* __STATUS_LOGGER_HPP__ */