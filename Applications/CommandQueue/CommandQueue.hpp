/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   CommandQueue.hpp
* @details  Command queue that can be processed by a processing object.
* @ingroup Applications
*******************************************************************************/
#ifndef __COMMAND_OBJECT_HPP__
#define __COMMAND_OBJECT_HPP__

//AbstractionLayer
#include "OperatingSystemModule.hpp"
//C++
#include <cstring>
#include <type_traits> //For making sure the LogicSignature type is compatible with the type you've set it to.

/**
 * @namespace CommandQueueTypes
 * @brief Types for the Queue of Responsibility CommandQueue
 */
namespace CommandQueueTypes {
    /// @typedef LogicSignature
    /// @brief A unique identifier for commands that processing objects can check for.
    using LogicSignature = uint32_t;

    /**
     * @struct Status
     * @brief The status of the Queue of Responsibility
     */
    struct Status {
        Count commandsQueued; ///< The number of commands currently queued.
    };

    /// @brief The maximum amount of commands that can be in a queue at the same time
    static constexpr Count MaxCommandQueueSize = 4;
    /// @brief The maximum amount of time to wait for a semaphore
    static constexpr Milliseconds SemaphoreTimeout = 1;
    /// @brief Tag for logging
    static constexpr char TAG[] = "CommandQueue";
}

/**
 * @class CommandQueue
 * @brief  Create a command and add data to a queue for later processing.
 * @details Commands can be uniquely accessed by name and type.
 * @note In case you're not aware, the static members are static only for classes of the same type.
 * @tparam name The name of the command.
 * @tparam T The type of data that the command can store.
 * @code
 * //Header file with command specifications
 * struct Command1 {
 *     static constexpr char Name[] = "Command1"
 *     using DataType = uint32_t;
 * };
 * 
 * //In some other source file, use the command specifications to create or access a command queue
 * CommandQueue<Command1::Name, Command1::DataType> commandQueue;
 * Command1::DataType commandData = 42;
 * commandQueue.addToQueue(commandData);
 * 
 * //Get the data later
 * Command1::DataType commandData;
 * ErrorType error = commandQueue.getNextInQueue(commandData);
 * @endcode 
 */
template<const char *name, typename T>
class CommandQueue {
    public:
    /**
     * @brief Constructor.
     * @details Initializes the binary semaphore for controlling concurrent access to the queue.
     */
    CommandQueue() {
        if (0 == strlen(_BinarySemaphore.data())) {
            _SemaphoreCount++;
            std::string semaphoreNumber = std::to_string(_SemaphoreCount);
            strncpy(_BinarySemaphore.data(), "commandQSem", OperatingSystemTypes::MaxSemaphoreNameLength);
            assert(strlen(_BinarySemaphore.data()) + semaphoreNumber.length() < OperatingSystemTypes::MaxSemaphoreNameLength);
            strncat(_BinarySemaphore.data(), semaphoreNumber.c_str(), semaphoreNumber.length());
            ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, _BinarySemaphore);
            assert(ErrorType::Success == error);

            _Commands.reserve(CommandQueueTypes::MaxCommandQueueSize);
            _Status.commandsQueued = 0;
        }
    }

    /**
     * @brief Add a command to the queue.
     * @param commandData The command data to be added to the queue.
     * @returns ErrorType::Success if the command was added to the queue
     * @returns ErrorType::Timeout if the semaphore times out
     * @returns ErrorType::LimitReached if the queue is full
     */
    ErrorType addToQueue() {
        ErrorType error = OperatingSystem::Instance().waitSemaphore(_BinarySemaphore, CommandQueueTypes::SemaphoreTimeout);
        if (ErrorType::Success != error) {
            return ErrorType::Timeout;
        }

        if (_Commands.size() >= CommandQueueTypes::MaxCommandQueueSize) {
            error = OperatingSystem::Instance().incrementSemaphore(_BinarySemaphore);
            assert(ErrorType::Success == error);
            return ErrorType::LimitReached;
        }

        _Commands.push_back(_data);

        error = OperatingSystem::Instance().incrementSemaphore(_BinarySemaphore);
        assert(ErrorType::Success == error);
        return ErrorType::Success;
    }

    /**
     * @brief Get the next command in the queue.
     * @param commandData The command data to be returned.
     * @returns ErrorType::Success if there is a command in the queue
     * @returns ErrorType::NoData if there are no commands in the queue
     * @returns Any errors returned by OperatingSystem::incrementSemaphore
     */
    ErrorType getNextInQueue(T &commandData) {
        ErrorType error = OperatingSystem::Instance().waitSemaphore(_BinarySemaphore, CommandQueueTypes::SemaphoreTimeout);

        if (ErrorType::Success == error) {
            if (ErrorType::Success == commandsInQueue()) {
                commandData = _Commands.front();
                _Commands.erase(_Commands.begin());

                error =  ErrorType::Success;
            }
            else {
                error = ErrorType::NoData;
            }

            ErrorType semaphoreError = OperatingSystem::Instance().incrementSemaphore(_BinarySemaphore);
            assert(ErrorType::Success == semaphoreError);
        }

        return error;
    }

    /**
     * @brief Check if there are any commands in the queue.
     * @returns ErrorType::Success if there are commands in the queue
     * @returns ErrorType::Negative otherwise.
     */
    ErrorType commandsInQueue() {
        return _Commands.empty() ? ErrorType::Negative : ErrorType::Success;
    }

    /// @brief Get a mutable reference to the data
    T &data() { return _data; }
    /// @brief Get a constant reference to the data
    const T &dataConst() const { return _data; }
    /// @brief Get the status as a constant reference
    const CommandQueueTypes::Status &status() const {
        _Status.commandsQueued = _Commands.size();
        return _Status;
    }

    private:
    /// @brief Unique number to append to the semaphore name
    inline static Count _SemaphoreCount = 0;
    /// @brief The queue of commands
    inline static std::vector<T> _Commands;
    /// @brief The status of the Queue of Responsibility
    inline static CommandQueueTypes::Status _Status;
    /// @brief The name of the semaphore
    inline static std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> _BinarySemaphore = {0};
    /// @brief The data stored in the command
    T _data;
};

#endif // __COMMAND_OBJECT_HPP__
