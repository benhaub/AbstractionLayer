//AbstractionLayer
#include "Error.hpp"
//ESP
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_wifi.h" //For wifi error codes.
#include "nvs.h"     //For NVS error codes.
#include "mbedtls/net_sockets.h"
#include <cerrno>
#include "lwip/netdb.h"

ErrorType fromPlatformError(int32_t err) {

    switch (err) {
        case ESP_FAIL:
        case EAI_FAIL:
        case ECONNABORTED:
        case HOST_NOT_FOUND:
            return ErrorType::Failure;
        case ESP_OK:
            return ErrorType::Success;
        case ESP_ERR_NO_MEM:
        case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
        case EAI_MEMORY:
            return ErrorType::NoMemory;
        case ESP_ERR_TIMEOUT:
            return ErrorType::Timeout;
        case ESP_ERR_NOT_SUPPORTED:
            return ErrorType::NotSupported;
        case ESP_ERR_WIFI_NOT_INIT:
        case ESP_ERR_NVS_NOT_INITIALIZED:
        case ESP_ERR_NVS_READ_ONLY:
        case ENOTCONN:
        case ECONNRESET:
            return ErrorType::PrerequisitesNotMet;
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_INVALID_SIZE:
        case ESP_ERR_NVS_VALUE_TOO_LONG:
        case ESP_ERR_NVS_KEY_TOO_LONG:
        case ESP_ERR_NVS_INVALID_NAME:
        case ESP_ERR_NVS_INVALID_LENGTH:
        case ESP_ERR_NVS_PART_NOT_FOUND:
        case EBADF:
        case EINVAL:
        case MBEDTLS_ERR_NET_UNKNOWN_HOST:
            return ErrorType::InvalidParameter;
        case ESP_ERR_NVS_NOT_FOUND:
            return ErrorType::FileNotFound;
        default:
            ESP_LOGW("EspError", "Got unhandled error code %ld", err);
            return ErrorType::Failure;
    }
}