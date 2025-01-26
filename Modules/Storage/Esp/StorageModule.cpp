//Modules
#include "StorageModule.hpp"
//ESP
#include "nvs.h"
#include "nvs_flash.h"

ErrorType Storage::initStorage() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(&Storage::initStorageInternal, this));
    return addEvent(event);
} 

ErrorType Storage::deinitStorage() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(&Storage::deinitStorageInternal, this));
    return addEvent(event);
} 

ErrorType Storage::maxStorageSize(Kilobytes &size, std::string partitionName) {
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
    
    return fromPlatformError(err);
}

ErrorType Storage::availableStorage(Kilobytes &size, std::string partitionName) {
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

    return fromPlatformError(err);
}

ErrorType Storage::erasePartition(const std::string &partitionName) {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<const std::string &>>(std::bind(&Storage::erasePartitionInternal, this, std::placeholders::_1), partitionName);
    return addEvent(event);
}

ErrorType Storage::eraseAllPartitions() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<>>(std::bind(&Storage::eraseAllPartitionsInternal, this));
    return addEvent(event);
}

ErrorType Storage::mainLoop() {
    return runNextEvent();
}

ErrorType Storage::initStorageInternal() {
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
    
ErrorType Storage::deinitStorageInternal() {
        if (_nvsHandle) {
            _nvsHandle.release();
        }

        return fromPlatformError(nvs_flash_deinit());
} 
    
ErrorType Storage::erasePartitionInternal(const std::string &partitionName) {
    return fromPlatformError(nvs_flash_erase_partition(partitionName.c_str()));
}

ErrorType Storage::eraseAllPartitionsInternal() {
    return fromPlatformError(_nvsHandle->erase_all());
}
