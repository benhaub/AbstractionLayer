/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   CommandQueue.hpp
* @details Command queue for serializing a desired action with the accompanying data.
* @ingroup Applications
*******************************************************************************/
#ifndef __COMMAND_OBJECT_HPP__
#define __COMMAND_OBJECT_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
#include "Math.hpp"
//C++
#include <atomic>
#include <cassert>

/**
 * @namespace CommandQueueTypes
 * @brief Types for the Queue of Responsibility CommandQueue
 */
namespace CommandQueueTypes {

    /**
     * @struct Status
     * @brief The status of the Queue of Responsibility
     */
    struct Status {
        Count commandsQueued; ///< The number of commands currently queued.
    };

    /// @brief The maximum amount of commands that can be in a queue at the same time
    static constexpr Count MaxCommandQueueSize = 8;
    /// @brief Tag for logging
    static constexpr char TAG[] = "CommandQueue";
}

/**
 * @class CommandQueue
 * @brief  Create a command and add data to a queue for later processing.
 * @details Command queue for serializing a desired action with the accompanying data.
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
     * @brief Add a command to the queue.
     * @param commandData The command data to be added to the queue.
     * @returns ErrorType::Success if the command was added to the queue
     * @returns ErrorType::LimitReached if the queue is full
     * @sa EventQueue::addEvent which has better inline documentation to explain the code.
     */
    ErrorType addToQueue() {
        ErrorType error = ErrorType::Failure;

        Count currentCommandQueueIndexLast = _CurrentCommandQueueIndexLast.load();
        while (!(_CurrentCommandQueueIndexLast.compare_exchange_weak(currentCommandQueueIndexLast, (currentCommandQueueIndexLast + 1) % _Commands.max_size())));
        if (CommandQueueNotFull()) {
            _Commands[currentCommandQueueIndexLast] = _data;
            _CommandsQueued.fetch_add(1, std::memory_order_relaxed);
            error = ErrorType::Success;
        }
        else {
            error = ErrorType::LimitReached;
        }

        return error;
    }

    /**
     * @brief Return and remove the next command in the queue.
     * @param commandData The command data to be returned.
     * @returns ErrorType::Success if there is a command in the queue
     * @returns ErrorType::NoData if there are no commands in the queue
     * @sa EventQueue::runNextEvent which has better inline documentation to explain the code.
     */
    ErrorType getNextInQueue(T &commandData) {
        ErrorType error = ErrorType::NoData;

        if (CommandsReady()) {
            commandData = _Commands[_CurrentCommandQueueIndexFirst];
            _CommandsQueued.fetch_sub(1, std::memory_order_relaxed);
            _CurrentCommandQueueIndexFirst = (_CurrentCommandQueueIndexFirst + 1) % _Commands.max_size();
            error = ErrorType::Success;
        }

        return error;
    }

    /**
     * @brief Return the next command in the queue without removing it
     * @param error The error that occurred while peaking the next command.
     * @returns The next command in th queue.
     * @post The returned command is valid only if error is ErrorType::Success
     * @post error == ErrorType::NoData if there are no commands in the queue to peak.
     */
    const T &peakNextInQueue(ErrorType &error) {
        error = ErrorType::NoData;

        if (CommandsReady()) {
            error = ErrorType::Success;
            return _Commands[_CurrentCommandQueueIndexFirst];
        }

        return _data;
    }

    /// @brief Get a mutable reference to the data
    T &data() { return _data; }
    /// @brief Get a constant reference to the data
    const T &dataConst() const { return _data; }
    /// @brief Get the status as a constant reference
    const CommandQueueTypes::Status &status() const {
        _Status.commandsQueued = _CommandsQueued.load();
        return _Status;
    }

    /**
     * @brief True when the command queue is not full 
     * @returns true if the command queue is not full
     * @returns false otherwise.
     */
    static bool CommandQueueNotFull() {
        return _CommandsQueued.load() < _Commands.max_size();
    }

    /**
     * @brief True when there are commands ready to read
     * @returns true if the command queue has commands ready to read
     * @returns false otherwise.
     */
    static bool CommandsReady() {
        return _CommandsQueued.load() > 0;
    }

    private:
    /// @brief The index of the next command to receive
    inline static Count _CurrentCommandQueueIndexFirst = 0;
    /// @brief The index of the last command to receive
    inline static std::atomic<Count> _CurrentCommandQueueIndexLast = 0;
    /// @brief the current count of commands queued.
    inline static std::atomic<Count> _CommandsQueued = 0;
    /// @brief The ring buffer queue of commands
    inline static std::array<T, CommandQueueTypes::MaxCommandQueueSize> _Commands;
    /// @brief The status of the Queue of Responsibility
    inline static CommandQueueTypes::Status _Status = {
        0
    };
    /// @brief The data stored in the command
    T _data;
};

#endif // __COMMAND_OBJECT_HPP__
