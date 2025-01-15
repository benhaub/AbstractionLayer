//Modules
#include "StorageModule.hpp"
//ESP
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"

ErrorType Storage::initStorage() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<Storage>>(std::bind(&Storage::initStorageInternal, this));
    return addEvent(event);
} 

ErrorType Storage::deinitStorage() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<Storage>>(std::bind(&Storage::deinitStorageInternal, this));
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

/**
 * @brief Returns the total size of the specified memory region.
 * @details It's generally not that useful to know the total ram size dynamically but rather the total size of the heap in ram.
 *          Static analysis of total RAM usage is preferred and can be done by summing up your .data and .bss sections and then adding
 *          on to that what your typical heap usage is during runtime.
 *          You can use idf.py size
 */
ErrorType Storage::maxRamSize(Kilobytes &size, std::string memoryRegionName) {
    if (memoryRegionName.empty()) {
        size = heap_caps_get_total_size(MALLOC_CAP_DEFAULT) / 1024;
    }
    else if (memoryRegionName == "DRAM") {
        size = heap_caps_get_total_size(MALLOC_CAP_8BIT) / 1024;
    }
    else if (memoryRegionName == "SPIRAM") {
        size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM) / 1024;
    }

    return ErrorType::Success;
}

/**
 * @brief Returns the available size of the specified memory region.
 * @sa maxRamSize
 */
ErrorType Storage::availableRam(Kilobytes &size, std::string memoryRegionName) {
    if (memoryRegionName.empty()) {
        size = heap_caps_get_free_size(MALLOC_CAP_DEFAULT) / 1024;
    }
    else if (memoryRegionName == "DRAM") {
        size = heap_caps_get_free_size(MALLOC_CAP_8BIT) / 1024;
    }
    else if (memoryRegionName == "SPIRAM") {
        size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024;
    }

    return ErrorType::Success;
}

ErrorType Storage::erasePartition(const std::string &partitionName) {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<Storage, const std::string &>>(std::bind(&Storage::erasePartitionInternal, this, std::placeholders::_1), partitionName);
    return addEvent(event);
}

ErrorType Storage::eraseAllPartitions() {
    std::unique_ptr<EventAbstraction> event = std::make_unique<Event<Storage>>(std::bind(&Storage::eraseAllPartitionsInternal, this));
    return addEvent(event);
}

ErrorType Storage::mainLoop() {
    ErrorType error;

    if (ErrorType::NoData == (error = runNextEvent())) {
        return error;
    }
    else if (ErrorType::Success == error) {
        return error;
    }

    return ErrorType::Failure;
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