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

/**
 * @class StatusLogger
 * @brief Periodic logging of all modules that offer a status
 */
class StatusLogger {

    public:
    /**
     * @brief Construct a new Status Logger object
     * @param interval Interval between logs
     * @pre Make sure the timer stack is large enough to be able to print.
     */
    StatusLogger(Seconds interval) {
        if (ErrorType::Success == OperatingSystem::Instance().createTimer(_logTimer, interval*1000, true, std::bind(&StatusLogger::printLog, this))) {
            if (ErrorType::Success != OperatingSystem::Instance().startTimer(_logTimer, 0)) {
                PLT_LOGW(TAG, "Failed to start timer for status logging");
            }

            _interval = interval;
        }
    }
    /**
     * @brief Deconstructor
     */
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

    /// @brief Toggle logging for a specified Abstraction
    ErrorType toggleLoggingFor(IpClientAbstraction *ipClient, bool toggleOn);
    ErrorType toggleLoggingFor(IpServerAbstraction *ipServer, bool toggleOn);
    ErrorType toggleLoggingFor(NetworkAbstraction *network, bool toggleOn);
    ErrorType toggleLoggingFor(OperatingSystemAbstraction *operatingSystem, bool toggleOn);
    ErrorType toggleLoggingFor(FileSystemAbstraction *filesystem, bool toggleOn);
    ErrorType toggleLoggingFor(StorageAbstraction *storage, bool toggleOn);

    private:
    /// @brief Interval between logs
    Seconds _interval = 60;
    /// @brief Timer ID
    Id _logTimer;

    /// @brief List of IpClients to log
    std::vector<IpClientAbstraction *> _ipClients = { };
    /// @brief List of IpServers to log
    std::vector<IpServerAbstraction *> _ipServers = { };
    /// @brief List of CellularNetworks to log
    std::vector<CellularAbstraction *> _cellularNetworks = { };
    /// @brief List of WifiNetworks to log
    std::vector<WifiAbstraction *> _wifiNetworks = { };
    //I hope for your sake that you are not working with two operating systems at the same time
    OperatingSystemAbstraction *_operatingSystem = nullptr;
    /// @brief List of FileSystems to log
    std::vector<FileSystemAbstraction *> _filesystems = { };
    /// @brief List of StorageMediums to log
    std::vector<StorageAbstraction *> _storageMediums = { };

    /// @brief Print the status of all registered Abstractions
    void printLog(void);
};
#endif /* __STATUS_LOGGER_HPP__ */