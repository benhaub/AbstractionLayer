//AbsractionLayer
#include "EventQueue.hpp"
#include "OperatingSystemModule.hpp"
#include "ProcessorModule.hpp"
#include "Math.hpp"
//C++
#include <cstring>

EventQueue::EventQueue() {
    //If the optimizations are disabled, the thread is not known to the OperatingSystem. It only knows about threads that it explicitely creates.
    //e.g. You create an event queue in main.cpp. This thread is not known to the OperatingSystem.
    _addEventOptimizationsEnabled = (ErrorType::Success == OperatingSystem::Instance().currentThreadId(_ownerThreadId));
}

ErrorType EventQueue::addEvent(Event &event) {
    Id currentThreadId = 0;
    OperatingSystem::Instance().currentThreadId(currentThreadId);

    if (_ownerThreadId == currentThreadId && _addEventOptimizationsEnabled) {
        return event.run();
    }
    else {
        Count currentEventIndexTail = _currentEventIndexTail.load();
        Count currentEventIndexHead = _currentEventIndexHead.load();
        if (eventQueueNotFull(currentEventIndexTail, currentEventIndexHead)) {
            //https://youtu.be/kPh8pod0-gk?list=PLc1ANd9mG2dwG-kovSjkjuWq8CpskvEye&t=1128
            while (!(_currentEventIndexTail.compare_exchange_weak(currentEventIndexTail, (currentEventIndexTail + 1) % events.max_size())));
        }
        else {
            return ErrorType::LimitReached;
        }

        events[(currentEventIndexTail + 1) % events.max_size()] = event;
    }

    return ErrorType::Success;
}

ErrorType EventQueue::runNextEvent() {
    ErrorType error = ErrorType::NoData;

    Count currentEventIndexTail = _currentEventIndexTail.load();
    Count currentEventIndexHead = _currentEventIndexHead.load();
    if (eventsReadyToRun(currentEventIndexTail, currentEventIndexHead)) {
        while (!(_currentEventIndexHead.compare_exchange_weak(currentEventIndexHead, (currentEventIndexHead + 1) % events.max_size())));
        error = events[(currentEventIndexHead + 1) % events.max_size()].run();
    }

    return error;
}

/// @brief Get the number of events available in the queue.
/// @return The number of events available in the queue.
Count EventQueue::eventsAvailable() const {
    Count currentEventIndexTail = _currentEventIndexTail.load();
    Count currentEventIndexHead = _currentEventIndexHead.load();
    return _MaxEvents - differenceBetween(currentEventIndexTail, currentEventIndexHead);
}

/// @brief True when the event queue is not full.
constexpr bool EventQueue::eventQueueNotFull(const Count &currentEventIndexTail, const Count &currentEventIndexHead) const {
    return differenceBetween(currentEventIndexTail, currentEventIndexHead) <= events.max_size();
}