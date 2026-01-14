/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   SignalsAndSlots.hpp
* @details Allows you to connect to signals which call a callback when emitted.
* @see https://doc.qt.io/qt-5/signalsandslots.html
* @see https://en.wikipedia.org/wiki/Observer_pattern
* @ingroup Applications
*******************************************************************************/
#ifndef __SIGNALS_AND_SLOTS_HPP__
#define __SIGNALS_AND_SLOTS_HPP__

//AbstractionLayer
#include "EventQueue.hpp"
//C++
#include <atomic>

/**
 * @namespace SignalsAndSlots
 * @brief Allows you to connect to signals which call a callback when emitted.
 * @code
 *     //Foo inherits from EventQueue.
 *     Foo foo;
 *     SignalsAndSlots::Signal<bool> bar;
 *     std::function<ErrorType(bool)> observerCallback = std::bind(&Foo::baz, &foo, std::placeholders::_1);
 *     //You pass in foo so that SignalsAndSlots knows what event queue to add the callback to
 *     bar.connect(&foo, observerCallback);
 *     //Or if foo connects itself via constructor or member function
 *     bar.connect(*this, std::bind(&Foo::baz, this, std::placeholders::_1));
 * 
 *     //The placeholder allows you to pass arguments to the callback function after the initial call to bind. 
 *     bar.emit(true);
 * @endcode
 */
namespace SignalsAndSlots {
    /**
     * @class Signal
     * @brief Adds a new signal
     * @tparam Args Optional arguments types that will be passed to observer callback functions
    */
    template <typename ...Args> class Signal {

        public:
        /**
         * @brief Observe a signal and call the callback when it is emitted
         * @details Interrupt and thread safe.
         * @param[in] callback The observers callback
         * @param[in] eventQueue The event queue to add the callback to
         * @param[out] handle The handle to the connection which can be used later to disconnect
         * @sa emit
         * @returns ErrorType::Success if the connection was successful
         * @returns ErrorType::LimitReached if the maximum number of observers has been reached
         * @returns ErrorType::Timeout if the semaphore could not be obtained in time
         * @returns ErrorType::Failure otherwise
         */
        ErrorType connect(EventQueue &eventQueue, std::function<ErrorType(Args...)> callback, Id &handle) {
            ErrorType error = ErrorType::LimitReached;

            auto itr = find_if(_slots.begin(), _slots.end(), [](const auto &slot) {
                return slot.first == nullptr;
            });

            if (itr != _slots.end()) {
                //The only other thing that can happen to _currentNumberOfObservers in the time after we load it is that it could be decremented by
                //disconnect() so if we pass this guard we will not overflow the maximum allowed number of observers.
                if (_currentNumberOfObservers.fetch_add(1, std::memory_order_relaxed) < _MaxNumberOfObservers) {
                    *itr = std::make_pair(&eventQueue, callback);
                    handle = std::distance(_slots.begin(), itr);
                    error = ErrorType::Success;
                }
                else {
                    _currentNumberOfObservers.fetch_sub(1, std::memory_order_relaxed);
                }
            }

            return error;
        }

        /**
         * @brief Emit the signal and notify all the observers who have connected themselves to a slot.
         * @details Interrupt and thread safe.
         * @sa connect
         * @returns ErrorType::NoData if there are no observers
         * @returns ErrorType::PrerequisitesNotMet if eventQueue is nullptr
         * @returns The errors described in SignalsAndSlots::Signal::_emit
        */
        ErrorType emit(const Args... args) {
            if (0 == _currentNumberOfObservers.load(std::memory_order_relaxed)) {
                return ErrorType::NoData;
            }

            //std::index_sequence_for<T...> takes a template parameter pack T...
            //It generates an std::index_sequence<0, 1, 2, ..., N-1>, where N is the number of types in the pack T...
            return _emit(std::forward_as_tuple(args...), std::index_sequence_for<Args...>());
        }

        /**
         * @brief disconnect a callback from this signal
         * @details Interrupt and thread safe.
         * @param handle The handle returned from the connect() which will disconnect the slot from further signal emissions.
         * @returns ErrorType::Success if the disconnection was successful or the handle was not found (already disconnected)
         * @returns ErrorType::Timeout if the semaphore could not be obtained in time
         * @returns ErrorType::Failure otherwise
         */
        ErrorType disconnect(const Id handle) {
            Id i = 0;
            auto itr = find_if(_slots.begin(), _slots.end(), [&i, handle](const auto &observer) {
                return i++ == handle;
            });

            if (itr != _slots.end()) {
                //If _currentNumberOfObservers is equal to _MaxNumberOfObservers, and we are interrupted or pre-empted before we can decrement
                //_currentNumberOfOberservers, then anything that tries to connect() will still see the queue as full.
                //If it is any other the value, then connect() will just select the next available spot.
                *itr = std::make_pair(nullptr, nullptr); 
                _currentNumberOfObservers.fetch_sub(1, std::memory_order_relaxed);
            }
            
            return ErrorType::Success;
        }

        private:
        /// @brief The max number of callbacks that can be added to a slot for every signal
        static constexpr Count _MaxNumberOfObservers = 16;
        /// @brief The current number of observers in the slots array.
        std::atomic<Count> _currentNumberOfObservers = 0;
        /// @brief List of all observer callbacks
        std::array<std::pair<EventQueue *, std::function<ErrorType(Args...)>>, _MaxNumberOfObservers> _slots = {std::make_pair(nullptr, nullptr)};

        /**
         * @brief Calls all of the observers with the callbacks they have registered using the connect() call
         * @details Interrupt safe and thread safe.
         * @sa SignalsAndSlots::connect
         * @returns ErrorType::Success if all observers had their callbacks queued to their event queues successfully.
         * @returns Any error returned by EventQueue::addEvent if one or more events failed. Only the error code of the
         *          last failure will be returned.
        */
        template <std::size_t... IndexSequence>
        ErrorType _emit([[maybe_unused]] const std::tuple<Args...> params, [[maybe_unused]] const std::index_sequence<IndexSequence...>) const {
            ErrorType error = ErrorType::NoData;

            for (auto &slot : _slots) {

                if (nullptr != slot.first && nullptr != slot.second) {
                    EventQueue &eventQueue = *(slot.first);
                    const std::function<ErrorType(Args...)> &callback = slot.second;

                    //The pack is IndexSequence, so the expansion is std::get<0>(params), std::get<1>(params), etc.
                    EventQueue::Event event = EventQueue::Event(std::bind(callback, std::get<IndexSequence>(params)...));
                    error = eventQueue.addEvent(event);
                }
            }

            return error;
        }
    };
};

#endif // __SIGNALS_AND_SLOTS_HPP__