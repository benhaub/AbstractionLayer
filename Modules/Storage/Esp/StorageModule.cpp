//AbstractionLayer
#include "StorageModule.hpp"
#include "OperatingSystemModule.hpp"
//ESP
#include "nvs.h"
#include "nvs_flash.h"

ErrorType Storage::initStorage() {
    if (_status.isInitialized) {
        return ErrorType::Success;
    }

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        //TODO: What should I do here? Try to ship off data to the cloud before resetting? Does Mark support that?
        return fromPlatformError(err);
    }

    _nvsHandle = nvs::open_nvs_handle(name().c_str(), NVS_READWRITE, &err);
    if (err != ESP_OK) {
        return fromPlatformError(err);
    }

    _status.isInitialized = true;
    return fromPlatformError(err);
} 

ErrorType Storage::deinitStorage() {
    bool deinitializationDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto deinitializedCallback = [&]() -> ErrorType {
        if (!_status.isInitialized) {
            deinitializationDone = true;
            callbackError = ErrorType::Success;
        }

        _nvsHandle.release();

        _status.isInitialized = false;
        deinitializationDone = true;
        callbackError = fromPlatformError(nvs_flash_deinit());
        return callbackError;
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
    auto maxStorageQueryDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto maxStorageQueryCallback = [&]() -> ErrorType {
        nvs_stats_t stats;
        esp_err_t err;

        if (partitionName.empty()) {
            err = nvs_get_stats(NULL, &stats);
        }
        else {
            err = nvs_get_stats(partitionName.c_str(), &stats);
        }

        //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#structure-of-a-page
        //One entry is 32 bytes.
        size = (stats.total_entries * 32) / 1024;

        callbackError = fromPlatformError(err);
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
        nvs_stats_t stats;
        esp_err_t err;

        if (partitionName.empty()) {
            err = nvs_get_stats(NULL, &stats);
        }
        else {
            err = nvs_get_stats(partitionName.c_str(), &stats);
        }

        //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#structure-of-a-page
        //One entry is 32 bytes.
        size = (stats.free_entries * 32) / 1024;

        callbackError = fromPlatformError(err);
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
        if (partitionName.empty()) {
            callbackError = fromPlatformError(nvs_flash_erase_partition(NULL));
        }
        else {
            callbackError = fromPlatformError(nvs_flash_erase_partition(partitionName.c_str()));
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
        callbackError = fromPlatformError(_nvsHandle->erase_all());
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