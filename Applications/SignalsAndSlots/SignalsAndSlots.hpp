/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   SignalAndSlots.hpp
* @details \b Synopsis: \n Allows you to connect to emitted signals which call a callback.
* @see https://doc.qt.io/qt-5/signalsandslots.html
* @see https://en.wikipedia.org/wiki/Observer_pattern
* @ingroup Applications
*******************************************************************************/
#ifndef __SIGNALS_AND_SLOTS_HPP__
#define __SIGNALS_AND_SLOTS_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
//TODO: I could move this to the cpp file if I create one.
#include "OperatingSystemModule.hpp"
#include "EventQueue.hpp"
//C++
#include <functional>
#include <memory>

/**
 * @namespace SignalAndSlots
 * @brief Allows you to connect to signals which call a callback when emitted.
 * @code
 *     auto observerCallback = [](bool complete, Milliseconds timestamp, const std::shared_ptr<std::string> ptr) {
 *         PLT_LOGI(TAG, "<Signal complete> <status:%s, message:%s, timestamp:%lld>", complete ? "true" : "false", ptr->c_str(), timestamp);
 *     };
 * 
 *     std::unique_ptr<SignalsAndSlots::SignalAbstraction> signal = std::make_unique<SignalsAndSlots::Signal<this, bool, std::shared_ptr<std::string>, Milliseconds>>(true, timestamp, ptr);
 * 
 *     bool complete = true;
 *     Milliseconds timestamp = 0;
 *     std::shared_ptr<std::string> ptr = std::make_shared<std::string>("Hello, world!");
 *  
 *     signal->connect(this, std::bind(observerCallback, true, timestamp, ptr));
 *     //
 *     // If the lambda was a member function declared as:
 *     // ErrorType Class::observerCallback(bool complete, Milliseconds timestamp, const std::shared_ptr<std::string> ptr) {
 *     //     return ErrorType::Success;
 *     // }
 *     // signal->connect(std::bind(&Class::observerCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true, timestamp, ptr));
 *     //
 */
namespace SignalsAndSlots {
    /// @brief The number of semaphores that have been created for all signals. Used to generate unique names.
    static int _SemaphoreCount = 0;

    /**
     * @class Signal
     * @brief Adds a new signal
     * @tparam Args Optional arguments types that will be passed to observer callback functions
    */
    template <typename ...Args> class Signal {

        public:
        /**
         * @brief Constructor
         * @tparam Args Optional arguments types that will be passed to observer callback functions
         */
        Signal(Args ...params) : _params(std::forward_as_tuple(params...)) {
            _SemaphoreCount++;
            _binarySemaphore = std::string("eventQueueBinarySemaphore").append(std::to_string(_SemaphoreCount));
            ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, _binarySemaphore);
            assert(ErrorType::Success == error);
        }
        ~Signal() = default;

        /**
         * @brief Observer a signal and call the callback when it is emitted
         * @param callback The observers callback
         * @param eventQueue The event queue to add the callback to
         * @sa emit
         * @returns ErrorType::Success if the connection was successful
         * @returns ErrorType::LimitReached if the maximum number of observers has been reached
         * @returns ErrorType::Timeout if the semaphore could not be obtained in time
         * @returns ErrorType::Failure otherwise
         */
        ErrorType connect(EventQueue &eventQueue, std::function<ErrorType(Args...)> callback) {
            if (_slots.size() == _MaxNumberOfObservers) {
                return ErrorType::LimitReached;
            }
            if (nullptr == callback.target()) {
                return ErrorType::InvalidParameter;
            }

            ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
            if (ErrorType::Success != error) {
                return ErrorType::Timeout;
            }

            _slots.push_back(std::make_pair(eventQueue, callback));

            error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
            assert(ErrorType::Success == error);

            return ErrorType::Success;
        }

        /**
         * @brief emit the signal and notify all the slots
         * @sa connect
         * @details convenience function so that you can call emit with no arguments which is more intuitive.
         * @returns ErrorType::NoData if there are no observers
         * @returns ErrorType::Success if the signal was emitted successfully
         * @returns ErrorType::Failure otherwise
        */
        ErrorType emit() {
            return _emit(_params, std::index_sequence_for<Args...>());
        }

        /**
         * @brief disconnect a callback from this signal
         * @param callback The observers callback to disconnect
         * @returns ErrorType::Success if the disconnection was successful
         * @returns ErrorType::Timeout if the semaphore could not be obtained in time
         * @returns ErrorType::Failure otherwise
         */
        ErrorType disconnect(std::function<ErrorType(Args...)> callback) {
            //if (nullptr == callback.target()) {
            //    return ErrorType::InvalidParameter;
            //}

            //ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
            //if (ErrorType::Success != error) {
            //    return ErrorType::Timeout;
            //}

            //std::erase_if(_slots, [&callback](const auto &observer) {
            //    if (nullptr != observer.target() && observer.second.target() == callback.target())
            //        return true;
            //    });
            
            //error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
            //assert(ErrorType::Success == error);

            return ErrorType::NotImplemented;
        }

        private:
        /// @brief The max number of callbacks that can be added to a slot for every signal
        static constexpr Count _MaxNumberOfObservers = 64;
        /// @brief The timeout for semaphore operations.
        static constexpr Milliseconds _SemaphoreTimeout = 0;

        /// @brief List of all observer callbacks
        std::vector<std::pair<EventQueue &, std::function<ErrorType(Args...)>>> _slots;
        /// @brief _params Tuple for forwarding parameter packs.
        std::tuple<Args...> _params;
        /// @brief The semaphore used to synchronize access to _slots.
        std::string _binarySemaphore;
        /// @brief the event queue for this signal.
        EventQueue *_eventQueue = nullptr;



        /**
         * @brief Calls all of the observers with the callbacks they have registered using the connect() call
         * @sa SignalsAndSlots::connect
         * @returns The error code of the function pointed to by the callback.
        */
        //TODO: If a class calls emit, then it will be responsible for executing all of the slot callbacks which is probably not what we want
        //The slots have to be executed on the thread that owns the observer callback. I think right now this class is not given enough information
        //to do that.
        //I think that if you are going to connect to a signal, you have to pass in your event queue so that this class can place the callback on
        //it for the thread to run later. If not, then maybe there could be some kind of option where the signal just sets a flag and it's up to the
        //observer to check the flag and do any necessary work.... but that's kind of just like the chain of responsibility class. I think the key
        //difference with the chain and with these signals and slots however is that the chain is first come, first serve. Whoever can process the
        //object does so and it is removed from the chain. In signals and slots, everyone gets a chance to process the event if it is observed.
        template <std::size_t... IndexSequence> ErrorType _emit(std::tuple<Args...> &params, std::index_sequence<IndexSequence...>) const {
            return ErrorType::NotImplemented;
        }
    };
};

#endif // __SIGNALS_AND_SLOTS_HPP__