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
        if (configTIMER_TASK_STACK_DEPTH >= 256) {
            if (ErrorType::Success == OperatingSystem::Instance().createTimer(_logTimer, interval*1000, true, std::bind(&StatusLogger::printLog, this))) {
                if (ErrorType::Success != OperatingSystem::Instance().startTimer(_logTimer, 0)) {
                    PLT_LOGW(TAG, "Failed to start timer for status logging");
                }

                _interval = interval;
            }
        }
        else {
            PLT_LOGI(TAG, "Timer stack too small for status logging");
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

    std::vector<IpClientAbstraction *> _ipClients = { };
    std::vector<IpServerAbstraction *> _ipServers = { };
    std::vector<CellularAbstraction *> _cellularNetworks = { };
    std::vector<WifiAbstraction *> _wifiNetworks = { };
    //I hope for your sake that you are not working with two operating systems at the same time
    OperatingSystemAbstraction *_operatingSystem = nullptr;
    std::vector<FileSystemAbstraction *> _filesystems = { };
    std::vector<StorageAbstraction *> _storageMediums = { };

    void printLog(void);
};
#endif /* __STATUS_LOGGER_HPP__ */