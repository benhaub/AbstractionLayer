//Modules
#include "StorageModule.hpp"
//AbstractionLayer
#include "Error.hpp"
//Common
#include "Log.hpp"
//Posix
#include <sys/statvfs.h>
#include <sys/stat.h>
//C++
#include <cstdio>

ErrorType Storage::initStorage() {
    ErrorType error;
    _rootPrefix = getEnvironment("HOME", error);
    _rootPrefix += "/fnddData";
    mkdir(_rootPrefix.c_str(), S_IRWXU);
    

    _status.isInitialized = true;
    return error;
} 

ErrorType Storage::deinitStorage() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<Storage>>(std::bind(&Storage::deinitStorageInternal, this));
    return addEvent(event);
} 

ErrorType Storage::maxStorageSize(Bytes &size, std::string partitionName) {
    struct statvfs fiData;
    ErrorType error = ErrorType::Success;

    if (0 == statvfs(rootPrefix().c_str(), &fiData)) {
        size = fiData.f_blocks * fiData.f_frsize;
    }
    else {
        error = fromPlatformError(errno);
        size = 0;
    }

    return error;
}

ErrorType Storage::availableStorage(Bytes &size, std::string partitionName) {
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
}

ErrorType Storage::maxRamSize(Bytes &size, std::string memoryRegionName) {
    ErrorType error = ErrorType::Failure;

    //Will return the size of RAM in bytes.
    std::string commandFinal("free -b | egrep Mem | tr -s \" \" | cut -d \" \" -f2");
    std::string ramSize(4, 0);
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(ramSize.data(), sizeof(uint8_t), ramSize.capacity(), pipe);
        if (feof(pipe) || bytesRead == ramSize.capacity()) {
            error = ErrorType::Success;
            ramSize.resize(bytesRead);
            while (ramSize.back() == '\n') {
                ramSize.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }
    size = std::strtoul(ramSize.c_str(), nullptr, 10);

    return error;
}

ErrorType Storage::availableRam(Bytes &size, std::string memoryRegionName) {
    ErrorType error = ErrorType::Failure;

    //Will return the size of available RAM in bytes.
    std::string commandFinal("free -b | egrep Mem | tr -s \" \" | cut -d \" \" -f4");
    std::string ramSize(4, 0);
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(ramSize.data(), sizeof(uint8_t), ramSize.capacity(), pipe);
        if (feof(pipe) || bytesRead == ramSize.capacity()) {
            error = ErrorType::Success;
            ramSize.resize(bytesRead);
            while (ramSize.back() == '\n') {
                ramSize.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }
    size = std::strtoul(ramSize.c_str(), nullptr, 10);

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