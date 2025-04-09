//AbstractionLayer
#include "StorageModule.hpp"
#include "OperatingSystemModule.hpp"
//ESP
#include "nvs.h"
#include "nvs_flash.h"

ErrorType Storage::init() {
    _status.isInitialized = true;
    return ErrorType::NotAvailable;
} 

ErrorType Storage::deinit() {
    _status.isInitialized = false;
    return ErrorType::NotAvailable;
}

ErrorType Storage::mainLoop() {
    return runNextEvent();
}