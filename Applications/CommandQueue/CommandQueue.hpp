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
#include "OperatingSystemModule.hpp"
//C++
#include <atomic>

#ifndef APP_MAX_NUMBER_OF_THREADS
#error APP_MAX_NUMBER_OF_THREADS must be defined so that the list of waiting threads is properly sized.
#endif

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

    /// @brief The list of threads waiting for commands to be added.
    using WaitingThreads = std::array<Id, APP_MAX_NUMBER_OF_THREADS>;
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

            for (auto &waitingThread : _WaitingThreads) {
                if (waitingThread != OperatingSystemTypes::NullId) {
                    OperatingSystem::Instance().unblock(waitingThread);
                }
            }

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
    const T &peakNextInQueue(ErrorType &error) const {
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
    bool commandsReady() const {
        return _CurrentCommandQueueIndexFirst.load() != _CurrentCommandQueueIndexLast.load();
    }

    /**
     * @brief add the thread Id given to this command queues waiting list.
     * @param thread The id of the thread to add to the waiting list.
     * @returns ErrorType::Success if the thread was added to the waiting list.
     * @returns ErrorType::LimitReached if the maximum number of waiting threads has been reached
     */
    ErrorType addToWaitingList(const Id thread) {
        ErrorType error = ErrorType::LimitReached;

        for (auto &waitingThread : _WaitingThreads) {

            if (waitingThread == OperatingSystemTypes::NullId) {
                waitingThread = thread;
                error = ErrorType::Success;
                break;
            }
        }

        return error;
    }
    
    /**
     * @brief Remove the thread Id given from this command queues waiting list.
     * @param threadId The id of the thread to remove from the waiting list.
     * @returns ErrorType::Success if the thread was removed from the waiting list.
     * @returns ErrorType::NoData if the thread was not found in the waiting list
     */
    ErrorType removeFromWaitingList(const Id threadId) {
        ErrorType error = ErrorType::NoData;

        for (auto &waitingThread : _WaitingThreads) {

            if (waitingThread == threadId) {
                waitingThread = OperatingSystemTypes::NullId;
                error = ErrorType::Success;
                break;
            }
        }

        return error;
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
    /// @brief List of waiting threads
    inline static CommandQueueTypes::WaitingThreads _WaitingThreads = {OperatingSystemTypes::NullId};

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

namespace {
    template <typename ...T>
    inline ErrorType AddToWaitingList(const Id thread) {
        const bool success = (... && (CommandQueue<T::Name, typename T::DataType>().addToWaitingList(thread) == ErrorType::Success));

        return success ? ErrorType::Success : ErrorType::LimitReached;
    }
}

namespace CommandQueueTypes {
    /**
     * @brief Concept to ensure that a type has the required Name and DataType members.
     * @tparam T The type to check.
     */
    template <typename T>
    concept HasNameAndDataType = requires {
        typename T::DataType;

        requires std::is_convertible_v<decltype(T::Name), const char *>;
    };
    /**
     * @brief Block until a command has been added to any of the given command queues.
     * @tparam A structure that contains the name of the command and the data type.
     * @returns Any errors returned by AddToWaitingList()
     * @returns Any errors returned by OperatingSystem::currentThreadId()
     * @returns Any errors returned by OperatingSystem::block()
     * @returns ErrorType::Success when commands are ready.
     * @post The thread will be removed from all waiting lists after it has been unblocked.
     * @code{.cpp}
     * struct SomeCommand1 {
     *    static constexpr char Name[] = "SomeCommand1"
     *    using DataType = uint32_t;
     * };
     * 
     * struct SomeCommand2 {
     *   static constexpr char Name[] = "SomeCommand2"
     *   using DataType = float;
     * };
     * 
     * waitForCommands<SomeCommand1, SomeCommand2>();
     * 
     * SomeCommand1::DataType data1;
     * SomeCommand2::DataType data2;
     * 
     * if (ErrorType::Success == command1.getNextInQueue(data1)) {
     *    //Process command1 data
     * }
     * else if (ErrorType::Success == command2.getNextInQueue(data2)) {
     *   //Process command2 data
     * }
     * @endcode
     */
    template <typename ...T>
    requires (... && HasNameAndDataType<T>)
    inline ErrorType WaitForCommands() {
        ErrorType error = ErrorType::Success;

        const bool noCommandsWaitingInQueues = (... && (CommandQueue<T::Name, typename T::DataType>().commandsReady() == false));

        if (noCommandsWaitingInQueues) {
            Id thread = OperatingSystemTypes::NullId;
            error = OperatingSystem::Instance().currentThreadId(thread);

            if (ErrorType::Success == error) {
                error = AddToWaitingList<T...>(thread);

                if (ErrorType::Success == error) {
                    error = OperatingSystem::Instance().block();
                }

                (((CommandQueue<T::Name, typename T::DataType>().removeFromWaitingList(thread)), ...));
            }
        }

        return error;
    }
}

#endif // __COMMAND_OBJECT_HPP__
