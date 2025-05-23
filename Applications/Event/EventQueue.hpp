/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   EventQueue.hpp
* @details \b Synopsis: \n Synchronization for calling function members of the
* base class.
* @see https://en.wikipedia.org/wiki/Reactor_pattern
* @ingroup Applications
*******************************************************************************/
#ifndef __EVENT_QUEUE_HPP__
#define __EVENT_QUEUE_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
//C++
#include <vector>
#include <tuple>
#include <functional>
#include <cassert>

/**
 * @class EventQueue
 * @details Queue is FIFO
 * @brief Provides an interface for synchronizing calls to base classes.
 * @post The current thread on which the event queue is created is the onwer thread.
 *       When subsequent events are called from this thread, the event queue will call them immediately
 *       when addEvent is called instead of queing them for runNextEvent.
 * @note Take care not to create the event queue from a thread that you do not intend to run the mainLoop from
 *       or the events will not be able to skip queing and will have to wait until runNextEvent is called. Remember
 *       that event queues are created at the point when any other class is created that inherits from EventQueue.
*/
class EventQueue {
    
    public:
    EventQueue();
    //TODO: This should be handled instead of asserting. Delete the queue when the last event is removed.
    ~EventQueue() { assert(events.size() == 0); };

    /// @brief Tag for logging.
    static constexpr char TAG[] = "EventQueue";

    /**
     * @class Event
     * @brief Runs the function and parameters passed to it by the constructor
    */
    class Event { 

        public:
        /**
         * @brief Constructor.
         * @param eventCallback The function member to call.
         * @post The eventCallback is not called.
         * @sa run
         * @code //Bind a function member. Can have any signature, any number of args.
         * ErrorType Class::functionMember(uint8_t arg1, uint32_t arg2, ErrorType &arg3) {
         *     return ErrorType::Success;
         * }
         * 
         * ErrorType Class::addEventToEventQueue(uint8_t arg1, uint32_t arg2, ErrorType &arg3) {
         *     EventQueue::Event event = EventQueue::Event(std::bind(&Class::functionMember, this, arg1, arg2, arg3));
         *     return addEvent(event);
         * 
         *     return ErrorType::Success;
         * }
         * @endcode
         * @code //Bind a lambda expression. Can have any signature, any number of args.
         * ErrorType Class::lambdaEvent(uint8_t arg1, uint32_t arg2, ErrorType &arg3, std::shared_ptr<uint8_t> ptr) {
         *     auto lambdaFunc = [](uint8_t arg1, uint32_t arg2, ErrorType &arg3, std::shared_ptr<uint8_t> ptr) -> ErrorType {
         *         return ErrorType::Success;
         *     };
         *   
         *   //Pass it off as a parameter to Event. Do not need to pass `this` as the first argument as we did in function members.
         *   EventQueue::Event event = EventQueue::Event(std::bind(lambdaFunc, arg1, arg2, arg3, ptr));
         * 
         *   return ErrorType::Success;
         * }
         * @endcode
        */
        Event(std::function<ErrorType()> eventCallback) : _eventCallback(eventCallback) {}

        /**
         * @brief Calls the function member with the parameters that were passed to the constructor.
        */
        ErrorType run() {
            return _eventCallback();

        }

        private:
        /// @brief The callback function of this event.
        std::function<ErrorType()> _eventCallback;
    };

    /**
     * @brief Adds an event to the to the queue.
     * @details Interrupt safe.
     * @param[in] event The event to add.
     * @post The event is added to a FIFO queue and will be executed when it reaches the first position in the queue and this thread
     *       is running.
     * @post Ownership of the event is transferred to the queue if, and only if, ErrorType::Success is returned.
     * @returns ErrorType::Success if the event was added
     * @returns ErrorType::LimitReached if the maximum number of events has been reached.
     * @returns ErrorType::Timeout if the semaphore could not be obtained in time
     * @returns the result of the event callback if the event is being added to from the same thread in which the event queue is run.
    */
    ErrorType addEvent(Event &event);

    /// @brief Get the number of events available in the queue.
    /// @return The number of events available in the queue.
    Count eventsAvailable() const { return _MaxEvents - events.size(); }

    /**
     * @brief The main loop for the eventQueue which can be used to continually check for and run events.
     * @sa runNextEvent
     * @returns ErrorType::NoData if there are no events to process.
     * @returns The error codes of any functions called by runNextEvent.
    */
    virtual ErrorType mainLoop() { return runNextEvent(); }

    protected: 
    /**
     * @fn runNextEvent
     * @brief Runs the next event in the queue.
     * @returns ErrorType::NoData if the queue is empty.
     * @returns ErrorType::Timeout if the semaphore could not be obtained in time
     * @returns The error code of the callback function pointed to by the Event.
    */
    ErrorType runNextEvent();

    private:
    /// @brief The maximum number of events that can be queued.
    static constexpr Count _MaxEvents = 10;
    /// @brief The timeout for semaphore operations.
    static constexpr Milliseconds _SemaphoreTimeout = 1;
    /// @brief the number of semaphores that have been created.
    static int _SemaphoreCount;
    /// @brief The queue of events to run.
    std::vector<Event> events;
    /// @brief The binary semaphore name for the next created semaphore.
    /// @details. Did not use the constant defined in OperatingSystemConfig since I don't want to inlcude the header for it. There is an assert to enforce this size instead.
    std::array<char, 16> _binarySemaphore;
    /// @brief The thread id of the owner of the event queue. Used to determine if we can skip event queuing.
    Id _ownerThreadId;
    /**
     * @brief True when optimizations for addEvents to your own event queue are enabled.
     * @details Optimization for when you add an event from the same thread that owns the event queue
     *          Instead of pushing the event on the queue for the mainLoop to run, just run it right away.
     */
    bool _addEventOptimizationsEnabled = false;

    /**
     * @brief Interrupt safe version of addEvent
     * @sa addEvent
     * @returns ErrorType::Success if the event was added
     * @returns ErrorType::LimitReached if the queue is full
     */
    ErrorType addEventFromIsr(Event &event);
};

#endif //__EVENT_QUEUE_HPP__
