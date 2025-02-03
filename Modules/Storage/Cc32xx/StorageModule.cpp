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
    ErrorType callbackError = ErrorType::Failure;

    auto deinitializedCallback = [&]() -> ErrorType {
        signed short result = sl_Stop(0xFFFF);
        if (0 == result) {
            _status.isInitialized = false;
        }

        return fromPlatformError(result);
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(deinitializedCallback));
    ErrorType error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!deinitializationDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType Storage::maxStorageSize(Kilobytes &size, std::string partitionName) {
    bool maxStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        switch (medium()) {
            case StorageTypes::Medium::Flash:
                callbackError = Flash::maxSize(size);
                break;
            default:
                break;
        }

        maxStorageQueryDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(maxStorageQueryCallback));
    ErrorType error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!maxStorageQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType Storage::availableStorage(Kilobytes &size, std::string partitionName) {
    bool availableStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto availableStorageQueryCallback = [&]() -> ErrorType {
        switch (medium()) {
            case StorageTypes::Medium::Flash:
                callbackError = Flash::available(size);
                break;
            default:
                break;
        }

        availableStorageQueryDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(availableStorageQueryCallback));
    ErrorType error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!availableStorageQueryDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType Storage::erasePartition(const std::string &partitionName) {
    bool erasePartitionDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto erasePartitionCallback = [&]() -> ErrorType {
        callbackError = ErrorType::NotSupported;

        switch (medium()) {
            case StorageTypes::Medium::Flash:
                callbackError = Flash::erasePartition(partitionName);
                break;
            default:
                break;
        }

        erasePartitionDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(erasePartitionCallback));
    ErrorType error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!erasePartitionDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType Storage::eraseAllPartitions() {
    bool eraseAllPartitionsDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto eraseAllPartitionsCallback = [&]() -> ErrorType {
        switch (medium()) {
            case StorageTypes::Medium::Flash:
                callbackError = Flash::eraseAllPartitions();
                break;
            default:
                break;
        }

        eraseAllPartitionsDone = true;
        return callbackError;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(eraseAllPartitionsCallback));
    ErrorType error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!eraseAllPartitionsDone) {
        OperatingSystem::Instance().delay(1);
    }

    return callbackError;
}

ErrorType Storage::mainLoop() {
    return runNextEvent();
}