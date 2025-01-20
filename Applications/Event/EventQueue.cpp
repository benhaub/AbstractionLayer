#include "EventQueue.hpp"
#include "OperatingSystemModule.hpp"

int EventQueue::_SemaphoreCount = 0;
EventQueue::EventQueue() {
    _SemaphoreCount++;
    _binarySemaphore = std::string("eventQueueBinarySemaphore").append(std::to_string(_SemaphoreCount));
    ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, _binarySemaphore);
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

    events.push_back(std::move(event));

    error = OperatingSystem::Instance().incrementSemaphore(_binarySemaphore);
    assert(ErrorType::Success == error);

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

    return ErrorType::Success;
}
