/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   StatusLogger.hpp
* @details Periodic logging of all modules that offer a status
* @ingroup Applications
*******************************************************************************/
#ifndef __STATUS_LOGGER_HPP__
#define __STATUS_LOGGER_HPP__

//AbstractionLayer
#include "Log.hpp"
#include "OperatingSystemModule.hpp"
#include "SignalsAndSlots.hpp"

#include "IpClientAbstraction.hpp"
#include "IpServerAbstraction.hpp"
#include "HttpServerAbstraction.hpp"
#include "NetworkAbstraction.hpp"
#include "FileSystemAbstraction.hpp"
#include "StorageAbstraction.hpp"
#include "CellularAbstraction.hpp"

namespace {
    /**
     * @concept CompatibleLoggers
     * @brief Concept for compatible loggers. Any calls that prints must be a base of an Abstraction listed below.
     * @details The concepts headers is not available with all compilers so the check to make sure the abstraction
     *          has a printStatus member function can only be done on some compilers.
     */
    template <typename Abstraction>
    concept CompatibleLoggers = requires(Abstraction abstraction) {
        std::is_base_of<IpClientAbstraction, Abstraction>::value ||
        std::is_base_of<CellularAbstraction, Abstraction>::value ||
        std::is_base_of<IpServerAbstraction, Abstraction>::value ||
        std::is_base_of<HttpServerAbstraction, Abstraction>::value ||
        std::is_base_of<NetworkAbstraction, Abstraction>::value ||
        std::is_base_of<OperatingSystemAbstraction, Abstraction>::value ||
        std::is_base_of<FileSystemAbstraction, Abstraction>::value ||
        std::is_base_of<StorageAbstraction, Abstraction>::value;
#if _GLIBCXX_CONCEPTS || _LIBCPP_STD_VER >= 20
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
     */
    StatusLogger(Seconds interval) {
        ErrorType error = ErrorType::Failure;
        if (ErrorType::Success == (error = OperatingSystem::Instance().createTimer(_logTimer, interval*1000, true, std::bind(&StatusLogger::timerElapsed, this)))) {
            if (ErrorType::Success == (error = OperatingSystem::Instance().startTimer(_logTimer, 0))) {
                _interval = interval;
            }
            else {
                PLT_LOGW(TAG, "Failed to start timer for status logging. <error:%u>", (uint8_t)error);
            }
        }
        else {
            PLT_LOGW(TAG, "Failed to create timer for status logging. <error:%u>", (uint8_t)error);
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

    /**
     * @class LoggerToggler
     * @brief Toggle logging for a specified Abstraction
     * @tparam Variable number of Abstraction types that can print a status.
     */
    template <typename... ListOfAbstractionsThatLog>
    /**
     * @class LoggerToggler
     * @brief Toggle logging on or off
     */
    class LoggerToggler {
        public:
        /// @brief StatusLogger needs to be able to access the list of abstractions.
        friend StatusLogger;

        /**
         * @brief Toggle logging for a specified Abstraction
         * @param abstraction Pointer to the Abstraction to toggle logging for
         * @param toggleOn True to enable logging, false to disable logging
         * @return Always returns ErrorType::Success
         */
        template <typename Abstraction>
        requires CompatibleLoggers<Abstraction>
        ErrorType toggleLoggingFor(Abstraction *abstraction, bool toggleOn) {
            //https://en.cppreference.com/w/cpp/utility/tuple/get
            //We get the list that holds the Abstraction given by giving the type to std::get.
            //https://youtu.be/gTNJXVmuRRA?list=PLc1ANd9mG2dwG-kovSjkjuWq8CpskvEye&t=1451
            auto &abstracionList = std::get<std::vector<Abstraction *>>(_loggers);

            if (toggleOn) {
                abstracionList.push_back(abstraction);
            }
            else {
                abstracionList.erase(std::remove(abstracionList.begin(), abstracionList.end(), abstraction), abstracionList.end());
            }

            return ErrorType::Success;
        }

        private:
        /// @brief List of Abstractions that will log a status periodically.
        std::tuple<std::vector<ListOfAbstractionsThatLog>...> _loggers;
    };
    /// @brief Used to toggle logging for an Abstraction
    LoggerToggler<IpClientAbstraction *, IpServerAbstraction *, HttpServerAbstraction *, NetworkAbstraction *, CellularAbstraction *, FileSystemAbstraction *, OperatingSystemAbstraction *, StorageAbstraction *> _loggerToggler;
    /// @brief Emitted when the logging interval has elapsed
    SignalsAndSlots::Signal<> _intervalElapsed;

    /// @brief Get the logging interval as a mutable reference
    Seconds &loggingInterval() { return _interval; }
    /// @brief Get the logging interval as a constant reference
    const Seconds &loggingInterval() const { return _interval; }

    /// @brief Print the status of all registered Abstractions
    void printLogs(void) {
        expandToListOfVectors(_loggerToggler._loggers);
    }

    private:
    /// @brief Interval between logs
    Seconds _interval = 60;
    /// @brief Timer ID
    Id _logTimer;

    /// @brief Emits a signal when the logging interval has elapsed so that users can call printLogs periodically.
    void timerElapsed(void) {
        _intervalElapsed.emit();
    }

    /**
     * @brief Expand a tuple of vectors into a list of vectors and print all the Abstractions that log in each vector.
     * @tparam AbstractionsThatLog. A list of Abstractions that can print a status
     * @param abstractions Tuple of vectors of Abstractions that can print a status
     * @details https://en.cppreference.com/w/cpp/utility/apply - Applies each element of the tuple to the function provided as an argument to it.
     */
    template<typename... AbstractionsThatLog>
    void expandToListOfVectors(std::tuple<AbstractionsThatLog...> &abstractions) {
        std::apply
        (
            [this](AbstractionsThatLog &... abstractions) -> void {
                //https://en.cppreference.com/w/cpp/language/pack
                //Pack expansion of tuple of vectors. Looks like: printStatus(std::vector<IpClientAbstraction *>); printStatus(std::vector<IpServerAbstraction *>), etc.
                (printStatus(abstractions), ...);
            },
            abstractions
        );
    }
    
    /**
     * @brief Print the status of the Abstraction given.
     * @tparam Abstraction. An Abstraction that can print a status
     * @param abstractions Vector of Abstractions that can print a status
     * @pre Must be CompatibleLogger
     * @sa CompatibleLoggers
     */
    template<typename Abstraction>
    requires CompatibleLoggers<Abstraction>
    void printStatus(std::vector<Abstraction *> &abstractions) {
        for (auto *abstraction : abstractions) {
            abstraction->printStatus();
        }
    }
};
#endif /* __STATUS_LOGGER_HPP__ */
