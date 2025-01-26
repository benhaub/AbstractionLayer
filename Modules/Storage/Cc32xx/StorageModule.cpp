//AbstractionLayer
#include "StorageModule.hpp"
#include "SpiModule.hpp"
#include "OperatingSystemModule.hpp"
//Cc32xx Storage module
#include "Flash.hpp"


ErrorType Storage::initStorage() {
    Spi spi;
    Id thread;
    ErrorType error = spi.init();
    if (ErrorType::Success != error) {
        return error;
    }

    constexpr Bytes kilobyte = 1024;
    //SIMPLELINK_THREAD_NAME is defined at compile time in cc32xx.cmake. Both Wifi and storage must create this thread in order to access the
    //API for Wifi and storage and may do so independently of each other. The Operating System must be check for this presence of this thread
    //before attempting to start it. Checking the return value of sl_start() is not sufficient since the wifi radio can be turned on or off at
    //any time.
    if (ErrorType::NoData == OperatingSystem::Instance().threadId(SIMPLELINK_THREAD_NAME, thread)) {
        
        error = OperatingSystem::Instance().createThread(OperatingSystemConfig::Priority::High,
                                                std::string(SIMPLELINK_THREAD_NAME),
                                                nullptr,
                                                2*kilobyte,
                                                sl_Task,
                                                thread);
        
        assert(ErrorType::Success == error);
    }

    _status.isInitialized = true;

    return error;
} 

ErrorType Storage::deinitStorage() {
    bool deinitializationDone = false;
    ErrorType error = ErrorType::Failure;

    auto deinitializedCallback = [this, &deinitializationDone, &error]() -> ErrorType {
        signed short result = sl_Stop(0xFFFF);
        if (0 == result) {
            _status.isInitialized = false;
        }

        return fromPlatformError(result);
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(deinitializedCallback));
    error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!deinitializationDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::maxStorageSize(Kilobytes &size, std::string partitionName) {
    bool maxStorageQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto maxStorageQueryCallback = [this, &maxStorageQueryDone, &error](Kilobytes &size, const std::string *partitionName) -> ErrorType {
        switch (medium()) {
            case StorageTypes::Medium::Flash:
                error = Flash::maxSize(size);
                break;
            default:
                break;
        }

        maxStorageQueryDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(maxStorageQueryCallback, size, &partitionName));
    error = addEvent(event);

    while (!maxStorageQueryDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::availableStorage(Kilobytes &size, std::string partitionName) {
    bool availableStorageQueryDone = false;
    ErrorType error = ErrorType::Failure;

    auto availableStorageQueryCallback = [this, &availableStorageQueryDone, &error](Kilobytes &size, const std::string *partitionName) -> ErrorType {
        switch (medium()) {
            case StorageTypes::Medium::Flash:
                error = Flash::available(size);
                break;
            default:
                break;
        }

        availableStorageQueryDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(availableStorageQueryCallback, size, &partitionName));
    error = addEvent(event);

    while (!availableStorageQueryDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::erasePartition(const std::string &partitionName) {
    bool erasePartitionDone = false;
    ErrorType error = ErrorType::Failure;

    auto erasePartitionCallback = [this, &erasePartitionDone, &error](const std::string *partitionName) -> ErrorType {
        ErrorType error = ErrorType::NotSupported;

        switch (medium()) {
            case StorageTypes::Medium::Flash:
                error = Flash::erasePartition(*partitionName);
                break;
            default:
                break;
        }

        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(erasePartitionCallback, &partitionName));
    error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!erasePartitionDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::eraseAllPartitions() {
    bool eraseAllPartitionsDone = false;
    ErrorType error = ErrorType::Failure;

    auto eraseAllPartitionsCallback = [this, &eraseAllPartitionsDone, &error]() -> ErrorType {
        switch (medium()) {
            case StorageTypes::Medium::Flash:
                error = Flash::eraseAllPartitions();
                break;
            default:
                break;
        }

        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(eraseAllPartitionsCallback));
    error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!eraseAllPartitionsDone) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType Storage::mainLoop() {
    return runNextEvent();
}