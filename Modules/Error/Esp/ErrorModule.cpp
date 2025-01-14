//AbstractionLayer
#include "Error.hpp"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h" //For wifi error codes.
#include "nvs.h"     //For NVS error codes.

ErrorType fromPlatformError(int32_t err) {

    switch (err) {
        case ESP_FAIL:
            return ErrorType::Failure;
        case ESP_OK:
            return ErrorType::Success;
        case ESP_ERR_NO_MEM:
            return ErrorType::NoMemory;
        case ESP_ERR_TIMEOUT:
            return ErrorType::Timeout;
        case ESP_ERR_NOT_SUPPORTED:
            return ErrorType::NotSupported;
        case ESP_ERR_WIFI_NOT_INIT:
            return ErrorType::PrerequisitesNotMet;
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_INVALID_SIZE:
            return ErrorType::InvalidParameter;
        case ESP_ERR_NVS_NOT_FOUND:
            return ErrorType::FileNotFound;
        default:
            ESP_LOGW("EspError", "Got unhandled error code %ld", err);
            return ErrorType::Failure;
    }
}