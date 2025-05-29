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
        //The last index must be allowed to go beyond the bounds of the array so that the last index can be filled with a value.
        //The index is always incremented for the next event to be added, not the current one.
        while (!(_currentEventQueueIndexLast.compare_exchange_weak(currentEventQueueIndexLast, (currentEventQueueIndexLast + 1) % _events.max_size())));
        if (eventQueueNotFull()) {
            //https://youtu.be/kPh8pod0-gk?list=PLc1ANd9mG2dwG-kovSjkjuWq8CpskvEye&t=1128
            //If the last index is the size of the array, then we know that what is stored in memory is now zero.
            _events[currentEventQueueIndexLast] = event;
            assert(_eventsQueued < _events.max_size());
            _eventsQueued++;
            error = ErrorType::Success;
        }
        else {
            error = ErrorType::LimitReached;
        }
    }

    return error;
}

ErrorType EventQueue::runNextEvent() {
    ErrorType error = ErrorType::NoData;

    if (eventsReady()) {
        //Copy the event so that the event callback doesn't block something else from adding an event until it has completed.
        Event nextEventToRun = _events[_currentEventQueueIndexFirst];

        //_currentEventQueueIndexFirst is not atomic since it is not needed to insert anything into the queue. There is a chance that addEvent() could be trying
        //to add to the queue while runNextEvent() is attempting to increment the head. addEvent() would see the queue as full and fail with LimitReached,
        //missing the update by microseconds. However an atomic would not help (much) since we have to copy out of the queue before we increment anyway to prevent
        //addEvent() from clobbering us before we run() the event so there is a pretty large window of time for addEvent to miss the update.
        _currentEventQueueIndexFirst = (_currentEventQueueIndexFirst + 1) % _events.max_size();
        assert(_eventsQueued > 0);
        _eventsQueued--;
        nextEventToRun.run();
    }

    return error;
}

bool EventQueue::eventQueueNotFull() {
    return _eventsQueued < _events.max_size();
}

bool EventQueue::eventsReady() {
    return _eventsQueued > 0;
}