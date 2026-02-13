//AbsractionLayer
#include "EventQueue.hpp"
#include "OperatingSystemModule.hpp"

EventQueue::EventQueue() {
    _ownerThreadId = OperatingSystemTypes::NullId;
    //If the optimizations are disabled, the thread is not known to the OperatingSystem. It only knows about threads that it explicitely creates.
    //e.g. You create an event queue in main.cpp. This thread is not known to the OperatingSystem.
    _addEventOptimizationsEnabled = (ErrorType::Success == OperatingSystem::Instance().currentThreadId(_ownerThreadId));
}

ErrorType EventQueue::addEvent(Event &event) {
    Id currentThreadId = 0;
    ErrorType error = OperatingSystem::Instance().currentThreadId(currentThreadId);

    //The event is being run from the same context as the thread that runs the mainLoop so we can skip the queue and run it immediately.
    if (_ownerThreadId == currentThreadId && _addEventOptimizationsEnabled) {
        error = event.run();
    }
    //Either the caller or the owner of the event queue are known to the operating system. Queuing would result in the event waiting in the queue forever
    //and the caller can not be blocked. Run the event immediately.
    else if (OperatingSystemTypes::NullId == currentThreadId || OperatingSystemTypes::NullId == _ownerThreadId) {
        error = event.run();
    }
    else if (addEventIfNotFull()) {
        Count currentEventQueueIndexLast = _currentEventQueueIndexLast.load();

        //https://youtu.be/kPh8pod0-gk?list=PLc1ANd9mG2dwG-kovSjkjuWq8CpskvEye&t=1128
        while (!(_currentEventQueueIndexLast.compare_exchange_weak(currentEventQueueIndexLast, (currentEventQueueIndexLast + 1) % _events.max_size())));

        //If the last index is the size of the array, then we know that what is stored in memory is now zero.
        _events[currentEventQueueIndexLast] = std::move(event);
        //_eventsReady is not a function because it needs to be evaluated at specific points in the code. We can't let runNextEvent try to run an
        //event while we are still in the middle of adding an event to the queue.
        _eventsReady = true;

        for (auto &waitingThread : _waitingThreads) {
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

ErrorType EventQueue::waitForEvents() {
    Id thread = OperatingSystemTypes::NullId;
    ErrorType threadIdError = OperatingSystem::Instance().currentThreadId(thread);
    ErrorType error = ErrorType::LimitReached;

    if (ErrorType::Success == threadIdError) {

        for (auto &waitingThread : _waitingThreads) {

            if (waitingThread == OperatingSystemTypes::NullId) {
                waitingThread = thread;
                error = OperatingSystem::Instance().block();
                waitingThread = OperatingSystemTypes::NullId;
                break;
            }
        }
    }

    return error;
}

ErrorType EventQueue::runNextEvent(const LoopMode loopMode) {
    ErrorType error = ErrorType::NoData;

    if (_eventsReady) {
        error = _events[_currentEventQueueIndexFirst].run();

        //https://youtu.be/ZQFzMfHIxng?t=3888
        _eventsClaimed.fetch_sub(1, std::memory_order_relaxed);

        //_currentEventQueueIndexFirst is not atomic since it is not needed to insert anything into the queue. There is a chance that addEvent() could be trying
        //to add to the queue while runNextEvent() is attempting to decrement the amount of events queued. addEvent() would see the queue as full and fail with LimitReached,
        //missing the update by microseconds.
        _currentEventQueueIndexFirst = (_currentEventQueueIndexFirst + 1) % _events.max_size();

        //There is no race condition between us and addEvent() because _currentEventQueueIndexLast will have already had it's value updated by the time
        //addEvent() is trying to update _eventsReady. We can either keep _eventsReady set to true, or set it to false and disable ourselves from running
        //attempting to run more events.
        _eventsReady = _currentEventQueueIndexFirst != _currentEventQueueIndexLast.load();
    }
    else if (loopMode == LoopMode::Blocking) {
        error = waitForEvents();
    }

    return error;
}

bool EventQueue::addEventIfNotFull() {
    //The only other thing that can happen to _eventsClaimed in the time after we load it is that it could be decremented by runNextEvent() so if we pass
    //this guard we will not overflow the queue.
    const Count eventsClaimed = _eventsClaimed.load();
    if (eventsClaimed < _events.max_size()) {
        _eventsClaimed.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    return false;
}
