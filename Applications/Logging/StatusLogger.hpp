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

#include "HttpServerAbstraction.hpp"
#include "IpClientAbstraction.hpp"
#include "IpServerAbstraction.hpp"
#include "NetworkAbstraction.hpp"
#include "FileSystemAbstraction.hpp"
#include "StorageAbstraction.hpp"

namespace {
    template <typename Abstraction>
    concept CompatibleLoggers = requires(Abstraction abstraction) {
        std::is_base_of<IpClientAbstraction, Abstraction>::value ||
        std::is_base_of<IpServerAbstraction, Abstraction>::value ||
        std::is_base_of<NetworkAbstraction, Abstraction>::value ||
        std::is_base_of<OperatingSystemAbstraction, Abstraction>::value ||
        std::is_base_of<FileSystemAbstraction, Abstraction>::value ||
        std::is_base_of<StorageAbstraction, Abstraction>::value;
#if _GLIBCXX_CONCEPTS
        { abstraction.printStatus() } -> std::same_as<void>;
#endif
    };
}

/**
 * @class StatusLogger
 * @brief Periodic logging of all modules that offer a status
 */
class StatusLogger {

    public:
    /**
     * @brief Construct a new Status Logger object
     * @param interval Interval between logs
     * @pre Make sure the timer stack is large enough to be able to print. Some tests have revealed the stack needs to be as large as 1.5kiB
     */
    StatusLogger(Seconds interval) {
        if (ErrorType::Success == OperatingSystem::Instance().createTimer(_logTimer, interval*1000, true, std::bind(&StatusLogger::printLogs, this))) {
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
    template <typename... ListOfAbstractionsThatLog>
    //How's that for a class name?
    class LoggerToggler {
        public:
        friend StatusLogger;

        template <typename Abstraction>
        requires CompatibleLoggers<Abstraction>
        ErrorType toggleLoggingFor(Abstraction *abstraction, bool toggleOn) {
            //https://en.cppreference.com/w/cpp/utility/tuple/get
            //We get the list that holds the Abstraction given by giving the type to std::get.
            //https://youtu.be/gTNJXVmuRRA?list=PLc1ANd9mG2dwG-kovSjkjuWq8CpskvEye&t=1451
            auto abstracionList = std::get<std::vector<Abstraction *>>(_loggers);

            if (toggleOn) {
                abstracionList.push_back(abstraction);
            }
            else {
                abstracionList.erase(std::remove(abstracionList.begin(), abstracionList.end(), abstraction), abstracionList.end());
            }

            return ErrorType::Success;
        }

        private:
        std::tuple<std::vector<ListOfAbstractionsThatLog>...> _loggers;
    };
    LoggerToggler<IpClientAbstraction *, IpServerAbstraction *, NetworkAbstraction *, FileSystemAbstraction *, OperatingSystemAbstraction *, StorageAbstraction *> _loggerToggler;

    private:
    /// @brief Interval between logs
    Seconds _interval = 60;
    /// @brief Timer ID
    Id _logTimer;

    /// @brief Print the status of all registered Abstractions
    //TODO: Doesn't look like this is printing.
    void printLogs(void) {
        expandToListOfVectors(_loggerToggler._loggers);
    }

    //https://en.cppreference.com/w/cpp/utility/apply
    template<typename... AbstractionsThatLog>
    void expandToListOfVectors(std::tuple<AbstractionsThatLog...> &abstractions) {
        std::apply
        (
            [this](AbstractionsThatLog &... abstractions) -> void {
                //https://en.cppreference.com/w/cpp/language/pack
                //Pack expansion. Looks like: printStatus(std::vector<IpClientAbstraction *>); printStatus(std::vector<IpServerAbstraction *>), etc.
                (printStatus(abstractions), ...);
            },
            abstractions
        );
    }

    template<typename Abstraction>
    void printStatus(std::vector<Abstraction *> &abstraction) {
        for (auto *abstraction : abstraction) {
            abstraction->printStatus();
        }
    }
};
#endif /* __STATUS_LOGGER_HPP__ */