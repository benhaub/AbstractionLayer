//AbstractionLayer
#include "Error.hpp"
#include "StorageModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Posix
#include <sys/statvfs.h>
#include <sys/stat.h>

ErrorType Storage::initStorage() {

    if (_status.isInitialized) {
        return ErrorType::Success;
    }

    ErrorType error;
    _rootPrefix = getEnvironment("HOME", error);
    _rootPrefix.append("/").append(name());
    mkdir(_rootPrefix.c_str(), S_IRWXU); 

    _status.isInitialized = true;
    return error;
} 

ErrorType Storage::deinitStorage() {
    return ErrorType::NotAvailable;
} 

ErrorType Storage::maxStorageSize(Bytes &size, std::string partitionName) {
    bool maxStorageQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto maxStorageQueryCallback = [this, &maxStorageQueryDone, &error](Bytes &size, const std::string *partitionName) -> ErrorType {
        struct statvfs fiData;

        if (0 == statvfs(rootPrefix().c_str(), &fiData)) {
            size = fiData.f_blocks * fiData.f_frsize;
        }
        else {
            error = fromPlatformError(errno);
            size = 0;
        }

        maxStorageQueryDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(maxStorageQueryCallback, size, &partitionName));
    error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!maxStorageQueryDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::availableStorage(Bytes &size, std::string partitionName) {
    bool availableStorageQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto availableStorageQueryCallback = [this, &availableStorageQueryDone, &error](Bytes &size, const std::string *partitionName) -> ErrorType {
        struct statvfs fiData;
        ErrorType error = ErrorType::Success;

        if (0 == statvfs(getEnvironment("HOME", error).c_str(), &fiData)) {
            size = fiData.f_bavail * fiData.f_frsize;
        }
        else {
            error = fromPlatformError(errno);
            size = 0;
        }

        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(availableStorageQueryCallback, size, &partitionName));
    error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!availableStorageQueryDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::erasePartition(const std::string &partitionName) {
    return ErrorType::NotImplemented;
}

ErrorType Storage::eraseAllPartitions() {
    return ErrorType::NotImplemented;
}

ErrorType Storage::mainLoop() {
    return runNextEvent();
}

std::string Storage::getEnvironment(std::string variable, ErrorType &error) {
    
    std::string environmentVariable(std::getenv(variable.c_str()));

    if (nullptr == environmentVariable.data()) {
        error = ErrorType::Failure;
        return environmentVariable;
    }
    else {
        error = ErrorType::Success;
        return environmentVariable;
    }
}