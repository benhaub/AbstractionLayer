/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   SignalsAndSlots.hpp
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
#include "OperatingSystemModule.hpp"
#include "EventQueue.hpp"
//C++
#include <functional>
#include <memory>

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
 * @endcode
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
        Signal() {
            _SemaphoreCount++;
            _binarySemaphore = std::string("eventQueueBinarySemaphore").append(std::to_string(_SemaphoreCount));
            ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, _binarySemaphore);
            assert(ErrorType::Success == error);
        }
        ~Signal() = default;

        /**
         * @brief Observe a signal and call the callback when it is emitted
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
            if (nullptr == callback) {
                return ErrorType::InvalidParameter;
            }

            ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
            if (ErrorType::Success != error) {
                return ErrorType::Timeout;
            }

            _slots.emplace_back(eventQueue, callback);

            error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
            assert(ErrorType::Success == error);

            return ErrorType::Success;
        }

        /**
         * @brief Emit the signal and notify all the observers who have connected themselves to a slot.
         * @sa connect
         * @returns ErrorType::NoData if there are no observers
         * @returns ErrorType::PrerequisitesNotMet if eventQueue is nullptr
         * @returns The errors described in SignalsAndSlots::Signal::_emit
        */
        ErrorType emit(const Args... args) {
            if (0 == _slots.size()) {
                return ErrorType::NoData;
            }

            return _emit(std::forward_as_tuple(args...), std::index_sequence_for<Args...>());
        }

        /**
         * @brief disconnect a callback from this signal
         * @param callback The observers callback to disconnect
         * @returns ErrorType::Success if the disconnection was successful
         * @returns ErrorType::Timeout if the semaphore could not be obtained in time
         * @returns ErrorType::Failure otherwise
         */
        ErrorType disconnect(std::function<ErrorType(Args...)> callback) {
            if (nullptr == callback.target()) {
                return ErrorType::InvalidParameter;
            }

            ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
            if (ErrorType::Success != error) {
                return ErrorType::Timeout;
            }

            std::erase_if(_slots, [&callback](const auto &observer) {
                if (nullptr != observer.target() && observer.second.target() == callback.target())
                    return true;
                });
            
            error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
            assert(ErrorType::Success == error);

            return ErrorType::Success;
        }

        private:
        /// @brief The max number of callbacks that can be added to a slot for every signal
        static constexpr Count _MaxNumberOfObservers = 64;
        /// @brief The timeout for semaphore operations.
        static constexpr Milliseconds _SemaphoreTimeout = 0;

        /// @brief List of all observer callbacks
        std::vector<std::pair<EventQueue &, std::function<ErrorType(Args...)>>> _slots;
        /// @brief The semaphore used to synchronize access to _slots.
        std::string _binarySemaphore;



        /**
         * @brief Calls all of the observers with the callbacks they have registered using the connect() call
         * @sa SignalsAndSlots::connect
         * @returns ErrorType::Success if all observers had their callbacks queued to their event queues successfully.
         * @returns Any error returned by EventQueue::addEvent if one or more events failed. Only the error code of the
         *          last failure will be returned.
        */
        template <std::size_t... IndexSequence>
        ErrorType _emit(const std::tuple<Args...> params, const std::index_sequence<IndexSequence...>) const {
            ErrorType returnError = ErrorType::Failure;

            for (const auto &slot : _slots) {
                EventQueue &eventQueue = slot.first;
                const std::function<ErrorType(Args...)> &callback = slot.second;

                std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(callback, std::get<IndexSequence>(params)...));
                assert(nullptr != event.get());
                ErrorType addEventError = eventQueue.addEvent(event);
                if (ErrorType::Success != addEventError) {
                    returnError = addEventError;
                }
            }

            return returnError;
        }
    };
};

#endif // __SIGNALS_AND_SLOTS_HPP__