//AbsractionLayer
#include "EventQueue.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <cstring>

int EventQueue::_SemaphoreCount = 0;

EventQueue::EventQueue() {
    _SemaphoreCount++;
    std::string semaphoreNumber = std::to_string(_SemaphoreCount);
    assert(_binarySemaphore.max_size() == OperatingSystemConfig::MaxSemaphoreNameLength);
    strncpy(_binarySemaphore.data(), "eventQSem", OperatingSystemConfig::MaxSemaphoreNameLength);
    assert(strlen(_binarySemaphore.data()) + semaphoreNumber.length() < OperatingSystemConfig::MaxSemaphoreNameLength);
    strncat(_binarySemaphore.data(), semaphoreNumber.c_str(), semaphoreNumber.length());
    ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, _binarySemaphore);
    assert(ErrorType::Success == error);
    error = OperatingSystem::Instance().currentThreadId(_ownerThreadId);
    assert(ErrorType::Success == error);
}

ErrorType EventQueue::addEvent(std::unique_ptr<EventAbstraction> &event) {

    ErrorType error = OperatingSystem::Instance().waitSemaphore(_binarySemaphore, _SemaphoreTimeout);
    if (ErrorType::Success != error) {
        return ErrorType::Timeout;
    }

    if (events.size() >= _maxEvents) {
        error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
        assert(ErrorType::Success == error);
        return ErrorType::LimitReached;
    }

    Id currentThreadId = 0;
    OperatingSystem::Instance().currentThreadId(currentThreadId);
    assert(ErrorType::Success == error);

    if (_ownerThreadId != currentThreadId) {
        events.push_back(std::move(event));
    }

    error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
    assert(ErrorType::Success == error);

    //Run the event outside of the sempahore protection so we don't block the event queue.
    if (_ownerThreadId == currentThreadId) {
        return event->run();
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

    auto event = std::move(events.front());
    assert(nullptr != event.get());
    events.erase(events.begin());

    error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
    assert(ErrorType::Success == error);

    //This needs to be run last, in case the event needs to add more events to the queue or run an event.
    error = event->run();

    return error;
}
