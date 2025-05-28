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
        _CommandAddedToQueue.store(false);
        while (!(_CurrentCommandQueueIndexLast.compare_exchange_weak(currentCommandQueueIndexLast, (currentCommandQueueIndexLast + 1) % (_Commands.max_size() + 1))));
        if (CommandQueueNotFull(currentCommandQueueIndexLast, _CurrentCommandQueueIndexFirst)) {
            _Commands[currentCommandQueueIndexLast % _Commands.max_size()] = _data;
            error = ErrorType::Success;
        }
        else {
            error = ErrorType::LimitReached;
        }

        _CommandAddedToQueue.store(true);

        return error;
    }

    /**
     * @brief Get the next command in the queue.
     * @param commandData The command data to be returned.
     * @returns ErrorType::Success if there is a command in the queue
     * @returns ErrorType::NoData if there are no commands in the queue
     * @sa EventQueue::runNextEvent which has better inline documentation to explain the code.
     */
    ErrorType getNextInQueue(T &commandData) {
        ErrorType error = ErrorType::NoData;

        const Count currentCommandQueueIndexFirst = _CurrentCommandQueueIndexLast.load();
        const bool commandAddedToQueue = _CommandAddedToQueue.load();
        if (commandAddedToQueue && CommandsReady(currentCommandQueueIndexFirst, _CurrentCommandQueueIndexLast)) {
            commandData = _Commands[_CurrentCommandQueueIndexLast % _Commands.max_size()];
            _CurrentCommandQueueIndexLast = (_CurrentCommandQueueIndexLast + 1) % (_Commands.max_size() + 1);
        }

        return error;
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

    /**
     * @brief Check if there are commands in the queue to read.
     * @returns True if there are commands ready to read in the queue
     * @returns false otherwise
     */
    static bool CommandsInQueue() {
        const Count currentCommandQueueIndexFirst = _CurrentCommandQueueIndexLast.load();
        return (CommandsReady(currentCommandQueueIndexFirst, _CurrentCommandQueueIndexLast));
    }

    private:
    /// @brief The index of the next command to receive
    inline static Count _CurrentCommandQueueIndexFirst = 0;
    /// @brief The index of the last command to receive
    inline static std::atomic<Count> _CurrentCommandQueueIndexLast = 0;
    /// @brief true when the event has been added to the queue.
    inline static std::atomic<bool> _CommandAddedToQueue = false;
    /// @brief The ring buffer queue of commands
    inline static std::array<T, CommandQueueTypes::MaxCommandQueueSize> _Commands;
    /// @brief The status of the Queue of Responsibility
    inline static CommandQueueTypes::Status _Status = {
        0
    };
    /// @brief The data stored in the command
    T _data;

    /**
     * @brief The number of commands queued that are ready to read
     * @param[in] currentCommandQueueIndexFirst The tail of the command queue.
     * @param[in] currentCommandQueueIndexHead The head of the command queue.
     * @returns The number of commands that are ready to read.
     */
    static Count CommandsQueued(const Count &currentCommandQueueIndexFirst, const Count &currentCommandQueueIndexHead) {
        return differenceBetween(currentCommandQueueIndexFirst, currentCommandQueueIndexHead, static_cast<Count>(_Commands.max_size()));
    }

    /**
     * @brief True when the command queue is not full 
     * @param[in] currentCommandQueueIndexFirst The tail of the command queue.
     * @param[in] currentCommandQueueIndexHead The head of the command queue.
     * @returns true if the command queue is not full
     * @returns false otherwise.
     */
    static bool CommandQueueNotFull(const Count &currentCommandQueueIndexFirst, const Count &currentCommandQueueIndexHead) {
        return CommandsQueued(currentCommandQueueIndexFirst, currentCommandQueueIndexHead) < _Commands.max_size();
    }

    /**
     * @brief True when there are commands ready to read
     * @param[in] currentCommandQueueIndexFirst The tail of the command queue.
     * @param[in] currentCommandQueueIndexHead The head of the command queue.
     * @returns true if the command queue has commands ready to read
     * @returns false otherwise.
     */
    static bool CommandsReady(const Count &currentCommandQueueIndexFirst, const Count &currentCommandQueueIndexHead) {
        return CommandsQueued(currentCommandQueueIndexFirst, currentCommandQueueIndexHead) > 0;
    }
};

#endif // __COMMAND_OBJECT_HPP__
