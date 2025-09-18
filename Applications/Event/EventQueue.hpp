/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   EventQueue.hpp
* @details Synchronization for calling function members of the
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
#include <array>
#include <functional>
#include <atomic>

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
        Event(std::function<ErrorType()> eventCallback = nullptr) {
            _eventCallback = eventCallback;
        }

        /**
         * @brief Calls the function member with the parameters that were passed to the constructor.
         * @post The eventCallback is set to nullptr and is invalidated. It can not be called again.
        */
        ErrorType run() {
            if (eventCallbackValid()) { 
                const ErrorType error = _eventCallback();
                _eventCallback = nullptr;
                return error;
            }
            else { [[unlikely]]
                return ErrorType::InvalidParameter;
            }

        }
        /**
         * @brief Returns true if the eventCallback is valid and can be called.
         * @returns true if the eventCallback is valid
         * @returns false if the eventCallback is not valid
        */
        bool eventCallbackValid() {
            if (_eventCallback) {
                return true;
            }
            else {
                return false;
            }
        }

        private:
        /// @brief The callback function of this event.
        std::function<ErrorType()> _eventCallback;
    };

    /**
     * @brief Adds an event to the to the queue.
     * @details Interrupt and thread safe.
     * @param[in] event The event to add.
     * @returns ErrorType::Success if the event was added
     * @returns ErrorType::LimitReached if the maximum number of events has been reached.
     * @returns the result of the event callback if the event is being added to from the same thread in which the event queue is run.
     * @post The event is added to a FIFO queue and will be executed when it reaches the first position in the queue and this thread
     *       is running.
     * @post If the owner of the event queue is blocked when the event is added, it will become unblocked after this call.
    */
    ErrorType addEvent(Event &event);

    /**
     * @brief The main loop for the eventQueue which can be used to continually check for and run events.
     * @sa runNextEvent
     * @returns ErrorType::NoData if there are no events to process.
     * @returns The error codes of any functions called by runNextEvent.
     * @post If you don't have any other processing to do outside of the event queue, you may choose to call OperatingSystem::block
     *       after this call returns since addEvent will unblock.
    */
    virtual ErrorType mainLoop() { return runNextEvent(); }

    /**
     * @brief If the event queue is not full, atomically increments the number of events queued.
     * @returns true if the event was added to the queue.
     * @returns false if the event queue is full.
     */
    bool addEventIfNotFull();

    protected: 
    /**
     * @brief Runs the next event in the queue.
     * @returns ErrorType::NoData if the queue is empty.
     * @returns The error code of the callback function pointed to by the Event.
     * @note This function is not thread safe and should not be called from an interrupt context not only because it is not reentrant but also because
     *       there is never a guarentee on whether the event will block or not.
    */
    ErrorType runNextEvent();

    private:
    /// @brief The maximum number of events that can be queued.
    static constexpr Count _MaxEvents = 10;
    /// @brief The ring buffer queue of events to run. 
    std::array<Event, _MaxEvents> _events;
    /// @brief The index of the next event to run.
    Count _currentEventQueueIndexFirst = 0;
    /// @brief The index of the last event to run.
    std::atomic<Count> _currentEventQueueIndexLast = 0;
    /// @brief The current number of of events claimed, but not saved to the queue.
    std::atomic<Count> _eventsClaimed = 0;
    /// @brief The thread id of the owner of the event queue. Used to determine if we can skip event queuing.
    Id _ownerThreadId;
    /// @brief True when there are events ready to read
    bool _eventsReady = false;
    /**
     * @brief True when optimizations for addEvents to your own event queue are enabled.
     * @details Optimization for when you add an event from the same thread that owns the event queue
     *          Instead of pushing the event on the queue for the mainLoop to run, just run it right away.
     */
    bool _addEventOptimizationsEnabled = false;
};

#endif //__EVENT_QUEUE_HPP__
