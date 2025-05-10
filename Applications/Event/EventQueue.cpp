//AbsractionLayer
#include "EventQueue.hpp"
#include "OperatingSystemModule.hpp"
#include "ProcessorModule.hpp"
//C++
#include <cstring>

int EventQueue::_SemaphoreCount = 0;

EventQueue::EventQueue() {
    _SemaphoreCount++;
    std::string semaphoreNumber = std::to_string(_SemaphoreCount);
    assert(_binarySemaphore.max_size() == OperatingSystemTypes::MaxSemaphoreNameLength);
    strncpy(_binarySemaphore.data(), "eventQSem", OperatingSystemTypes::MaxSemaphoreNameLength);
    assert(strlen(_binarySemaphore.data()) + semaphoreNumber.length() < OperatingSystemTypes::MaxSemaphoreNameLength);
    strncat(_binarySemaphore.data(), semaphoreNumber.c_str(), semaphoreNumber.length());
    ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, _binarySemaphore);
    assert(ErrorType::Success == error);
    error = OperatingSystem::Instance().currentThreadId(_ownerThreadId);
    //If assert fails, the thread is not known to the OperatingSystem. It only knows about threads that it explicitely creates.
    //e.g. You create an event queue in main.cpp. This thread is not known to the OperatingSystem.
    assert(ErrorType::Success == error);
}

ErrorType EventQueue::addEventFromIsr(Event &event) {
    if (events.size() >= _MaxEvents) {
        return ErrorType::LimitReached;
    }

    //If we are preempted during push_back, then the higher priorty interrupt may also call push_back
    //and corrupt the queue.
    ErrorType error;
    if (ErrorType::Success != (error = OperatingSystem::Instance().disableAllInterrupts())) {
        const bool isCriticalError = !(ErrorType::NotAvailable == error);
        if (isCriticalError) {
            assert(false);
        }
    }
    events.push_back(event);
    if (ErrorType::Success != OperatingSystem::Instance().enableAllInterrupts()) {
        const bool isCriticalError = !(ErrorType::NotAvailable == error);
        if (isCriticalError) {
            assert(false);
        }
    }
    return ErrorType::Success;
}

ErrorType EventQueue::addEvent(Event &event) {
    if (ErrorType::Success == Processor::Instance().isInterruptContext()) {
        return addEventFromIsr(event);
    }

    ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
    if (ErrorType::Success != error) {
        return ErrorType::Timeout;
    }

    if (events.size() >= _MaxEvents) {
        error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
        assert(ErrorType::Success == error);
        return ErrorType::LimitReached;
    }

    Id currentThreadId = 0;
    OperatingSystem::Instance().currentThreadId(currentThreadId);
    assert(ErrorType::Success == error);

    //Optimization for when you add an event from the same thread that owns the event queue
    //Instead of pushing the event on the queue for the mainLoop to run, just run it right away.
    if (_ownerThreadId != currentThreadId) {
        ErrorType error;
        //Guarentee that addEventFromIsr will not corrupt the queue when it tries to add to it.
        if (ErrorType::Success != (error = OperatingSystem::Instance().disableAllInterrupts())) {
            const bool isCriticalError = !(ErrorType::NotAvailable == error);
            if (isCriticalError) {
                assert(false);
            }
        }
        events.push_back(event);
        if (ErrorType::Success != OperatingSystem::Instance().enableAllInterrupts()) {
            const bool isCriticalError = !(ErrorType::NotAvailable == error);
            if (isCriticalError) {
                assert(false);
            }
        }
    }

    error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
    assert(ErrorType::Success == error);

    //Run the event outside of the sempahore protection so we don't block the event queue.
    if (_ownerThreadId == currentThreadId) {
        return event.run();
    }

    return ErrorType::Success;
}

ErrorType EventQueue::runNextEvent() {
    ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
    if (ErrorType::Success != error) {
        return ErrorType::Timeout;
    }

    if (0 == events.size()) {
        error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
        assert(ErrorType::Success == error);
        return ErrorType::NoData;
    }

    Event event = events.front();
    events.erase(events.begin());

    error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
    assert(ErrorType::Success == error);

    //This needs to be run last, in case the event needs to add more events to the queue or run an event.
    error = event.run();

    return error;
}
