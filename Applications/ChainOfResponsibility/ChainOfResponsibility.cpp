//Foundation
#include "ChainOfResponsibility.hpp"
#include "OperatingSystemModule.hpp"

ChainOfResponsibility::ChainOfResponsibility() {
    binarySemaphore = std::string("chainBinarySemaphore");

    ErrorType error = OperatingSystem::Instance().createSemaphore(1, 1, binarySemaphore);
    assert(ErrorType::Success == error);

    _status.commandObjectCount = 0;
}

ErrorType ChainOfResponsibility::addCommandObject(std::unique_ptr<CommandObject> &commandObject) {
    assert(nullptr != commandObject.get());

    ErrorType error = OperatingSystem::Instance().waitSemaphore(binarySemaphore, SemaphoreTimeout);
    if (ErrorType::Success != error) {
        return ErrorType::Timeout;
    }

    LogicSignature logicSignature = commandObject->logicSignature();

    if (_commandObjects[logicSignature].size() >= MaxCommandObjectSize) {
        error = OperatingSystem::Instance().incrementSemaphore(binarySemaphore);
        assert(ErrorType::Success == error);
        return ErrorType::LimitReached;
    }

    assert(nullptr != commandObject.get());
    _commandObjects[logicSignature].push_back(std::move(commandObject));

    //commandObject is now nullptr after being moved to the vector. The vector owns the command object now
    //and it will be deleted when it is removed from the vector.
    error = OperatingSystem::Instance().incrementSemaphore(binarySemaphore);
    assert(ErrorType::Success == error);
    return ErrorType::Success;
}

std::unique_ptr<CommandObject> ChainOfResponsibility::getNextCommand(LogicSignature signature, ErrorType &error) {
    error = OperatingSystem::Instance().waitSemaphore(binarySemaphore, SemaphoreTimeout);
    if (ErrorType::Success != error) {
        error = ErrorType::Timeout;
        return nullptr;
    }

    if (isCommandWaiting(signature)) {
        auto command = std::move(_commandObjects[signature].front());
        _commandObjects[signature].erase(_commandObjects[signature].begin());
        error = OperatingSystem::Instance().incrementSemaphore(binarySemaphore);
        assert(ErrorType::Success == error);
        error = ErrorType::Success;
        return command;
    }

    error = OperatingSystem::Instance().incrementSemaphore(binarySemaphore);
    assert(ErrorType::Success == error);
    error = ErrorType::NoData;
    return nullptr;
}

bool ChainOfResponsibility::isCommandWaiting(LogicSignature signature) {
    auto commandObjectQueue = _commandObjects.find(signature);

    const bool commandObjectQueueWithSignatureExists = (commandObjectQueue != _commandObjects.end());

    if (commandObjectQueueWithSignatureExists) {
        const bool theQueueIsNotEmpty = (0 != commandObjectQueue->second.size());
        if (theQueueIsNotEmpty) {
            return true;
        }
    }

    return false;
}
