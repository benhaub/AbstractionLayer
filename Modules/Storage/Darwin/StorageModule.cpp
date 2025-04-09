//AbstractionLayer
#include "Error.hpp"
#include "StorageModule.hpp"
#include "Log.hpp"
//Posix
#include <sys/statvfs.h>
#include <sys/stat.h>

ErrorType Storage::initStorage() {
    ErrorType error;
    _rootPrefix = getEnvironment("HOME", error);
    _rootPrefix += "/" + name();
    mkdir(_rootPrefix.c_str(), S_IRWXU);
    

    _status.isInitialized = true;
    return error;
} 

ErrorType Storage::deinitStorage() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<Storage>>(std::bind(&Storage::deinitStorageInternal, this));
    return addEvent(event);
} 

ErrorType Storage::maxStorageSize(Kilobytes &size, std::string partitionName) {
    struct statvfs fiData;
    ErrorType error = ErrorType::Success;

    if (0 == statvfs(rootPrefix().c_str(), &fiData)) {
        size = fiData.f_blocks * (fiData.f_frsize / 1024);
    }
    else {
        error = fromPlatformError(errno);
        size = 0;
    }

    return error;
}

ErrorType Storage::availableStorage(Kilobytes &size, std::string partitionName) {
    struct statvfs fiData;
    ErrorType error = ErrorType::Success;

    if (0 == statvfs(getEnvironment("HOME", error).c_str(), &fiData)) {
        size = fiData.f_bavail * (fiData.f_frsize / 1024);
    }
    else {
        error = fromPlatformError(errno);
        size = 0;
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

ErrorType Storage::deinitStorageInternal() {
    //No storage deinit on stdlib systems. Assume the storage is already deinitialized.
    return ErrorType::NotAvailable;
}

ErrorType Storage::erasePartitionInternal(const std::string &partitionName) {
    return ErrorType::NotImplemented;
}

ErrorType Storage::eraseAllPartitionsInternal() {
    return ErrorType::NotImplemented;
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
