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
 * @sa EventQueue which has better inline documentation to explain the code.
 */
template<const char *name, typename T>
class CommandQueue {

    public:
    /**
     * @brief Add a command to the queue.
     * @details Interrupt and thread safe.
     * @param commandData The command data to be added to the queue.
     * @returns ErrorType::Success if the command was added to the queue
     * @returns ErrorType::LimitReached if the queue is full
     */
    ErrorType addToQueue(T &commandData) {
        ErrorType error = ErrorType::Failure;

        if (addCommandIfNotFull()) {
            Count currentCommandQueueIndexLast = _CurrentCommandQueueIndexLast.load();

            while (!(_CurrentCommandQueueIndexLast.compare_exchange_weak(currentCommandQueueIndexLast, (currentCommandQueueIndexLast + 1) % _Commands.max_size())));

            _Commands[currentCommandQueueIndexLast] = std::move(commandData);
            _CommandsReady = true;
            error = ErrorType::Success;
        }
        else {
            error = ErrorType::LimitReached;
        }

        return error;
    }

    /**
     * @brief Return and remove the next command in the queue.
     * @details Interrupt and thread safe.
     * @param commandData The command data to be returned.
     * @returns ErrorType::Success if there is a command in the queue
     * @returns ErrorType::NoData if there are no commands in the queue
     */
    ErrorType getNextInQueue(T &commandData) {
        ErrorType error = ErrorType::NoData;

        if (_CommandsReady) {
            Count currentCommandQueueIndexFirst = _CurrentCommandQueueIndexFirst.load();

            while ((_CommandsReady = (currentCommandQueueIndexFirst != _CurrentCommandQueueIndexLast.load())) && !(_CurrentCommandQueueIndexFirst.compare_exchange_weak(currentCommandQueueIndexFirst, (currentCommandQueueIndexFirst + 1) % _Commands.max_size())));

            if (_CommandsReady) {
                commandData = std::move(_Commands[currentCommandQueueIndexFirst]);
                _CommandsClaimed.fetch_sub(1, std::memory_order_relaxed);

                error = ErrorType::Success;
            }
        }

        return error;
    }

    /**
     * @brief Return the next command in the queue without removing it
     * @param error The error that occurred while peaking the next command.
     * @returns The next command in th queue.
     * @post The returned command is valid only if error is ErrorType::Success
     * @post error == ErrorType::NoData if there are no commands in the queue to peak. The data returns will be whatever is in the current index.
     */
    const T &peakNextInQueue(ErrorType &error) {
        const Count currentCommandQueueIndexFirst = _CurrentCommandQueueIndexFirst.load();
        if (currentCommandQueueIndexFirst != _CurrentCommandQueueIndexLast.load()) {
            error = ErrorType::Success;
        }

        return _Commands[currentCommandQueueIndexFirst];
    }

    /**
     * @brief Check if there are commands ready.
     * @details Yes, it's true that either of these atomics could changed immediately after this call. The point of this function is to just try to save
     *          the caller the burden of any processing that might be incurred if there are commands in the queue. At the end of the day, if there isn't
     *          a command in the queue when getNextInQueue() is called, you will receive ErrorType::NoData.
     * @returns true if there are commands in the queue
     * @returns false otherwise.
     */
    bool commandsReady() {
        return _CurrentCommandQueueIndexFirst.load() != _CurrentCommandQueueIndexLast.load();
    }

    /// @brief Get the status as a constant reference
    const CommandQueueTypes::Status &status() const {
        _Status.commandsQueued = _CommandsClaimed.load();
        return _Status;
    }

    private:
    /// @brief The index of the next command to receive
    inline static std::atomic<Count> _CurrentCommandQueueIndexFirst = 0;
    /// @brief The index of the last command to receive
    inline static std::atomic<Count> _CurrentCommandQueueIndexLast = 0;
    /// @brief the current count of commands claimed, but not saved to the queue.
    inline static std::atomic<Count> _CommandsClaimed = 0;
    /// @brief The ring buffer queue of commands
    inline static std::array<T, CommandQueueTypes::MaxCommandQueueSize> _Commands;
    /// @brief The status of the Queue of Responsibility
    inline static CommandQueueTypes::Status _Status = {
        0
    };
    inline static bool _CommandsReady = false;

    bool addCommandIfNotFull() {
        //The only other thing that can happen to _eventsClaimed in the time after we load it is that it could be decremented by runNextEvent() so if we pass
        //this guard we will not overflow the queue.
        const Count commandsClaimed = _CommandsClaimed.load();
        if (commandsClaimed < _Commands.max_size()) {
            _CommandsClaimed.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        return false;
    }
};

#endif // __COMMAND_OBJECT_HPP__
