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
    ErrorType error = ErrorType::Failure;
    OperatingSystem::Instance().currentThreadId(currentThreadId);

    if (_ownerThreadId == currentThreadId && _addEventOptimizationsEnabled) {
        error = event.run();
    }
    else {
        Count currentEventQueueIndexLast = _currentEventQueueIndexLast.load();
        //As soon as we increment that last index count, runNextEvent has free reign, so we have to finish this entire operation.
        _eventAddedToQueue.store(false);
        //The last index must be allowed to go beyond the bounds of the array so that the last index can be filled with a value.
        //The index is always incremented for the next event to be added, not the current one.
        while (!(_currentEventQueueIndexLast.compare_exchange_weak(currentEventQueueIndexLast, (currentEventQueueIndexLast + 1) % (_events.max_size() + 1))));
        if (eventQueueNotFull(currentEventQueueIndexLast, _currentEventQueueIndexFirst)) {
            //https://youtu.be/kPh8pod0-gk?list=PLc1ANd9mG2dwG-kovSjkjuWq8CpskvEye&t=1128
            //If the last index is the size of the array, then we know that what is stored in memory is now zero.
            _events[currentEventQueueIndexLast % _events.max_size()] = event;
            error = ErrorType::Success;
        }
        else {
            error = ErrorType::LimitReached;
        }
    }

    _eventAddedToQueue.store(true);

    return error;
}

ErrorType EventQueue::runNextEvent() {
    ErrorType error = ErrorType::NoData;

    const Count currentEventQueueIndexLast = _currentEventQueueIndexLast.load();
    const bool eventAddedToQueue = _eventAddedToQueue.load();
    if (eventAddedToQueue && eventsReady(currentEventQueueIndexLast, _currentEventQueueIndexFirst)) {
        //Copy the event so that the event callback doesn't block something else from adding an event until it has completed.
        Event nextEventToRun = _events[_currentEventQueueIndexFirst % _events.max_size()];

        //_currentEventQueueIndexFirst is not atomic since it is not needed to insert anything into the queue. There is a chance that addEvent() could be trying
        //to add to the queue while runNextEvent() is attempting to increment the head. addEvent() would see the queue as full and fail with LimitReached,
        //missing the update by microseconds. However an atomic would not help (much) since we have to copy out of the queue before we increment anyway to prevent
        //addEvent() from clobbering us before we run() the event so there is a pretty large window of time for addEvent to miss the update.

        //We allow the first index to reach max_size() + 1 so that it doesn't reset to 0 while the tail is at 10 which gives the impression that the queue is full.
        _currentEventQueueIndexFirst = (_currentEventQueueIndexFirst + 1) % (_events.max_size() + 1);
        nextEventToRun.run();
    }

    return error;
}

Count EventQueue::eventsQueued(const Count &currenteventQueueIndexTail, const Count &currenteventQueueIndexHead) {
    return differenceBetween(currenteventQueueIndexTail, currenteventQueueIndexHead, static_cast<Count>(_events.max_size()));
}

bool EventQueue::eventQueueNotFull(const Count &currenteventQueueIndexTail, const Count &currenteventQueueIndexHead) {
    return eventsQueued(currenteventQueueIndexTail, currenteventQueueIndexHead) < _events.max_size();
}

bool EventQueue::eventsReady(const Count &currenteventQueueIndexTail, const Count &currenteventQueueIndexHead) {
    return eventsQueued(currenteventQueueIndexTail, currenteventQueueIndexHead) > 0;
}