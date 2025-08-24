//AbstractionLayer
#include "Error.hpp"
#include "StorageModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Posix
#include <sys/statvfs.h>
#include <sys/stat.h>
//C++
#include <cstring>

#ifndef APP_HOME_DIRECTORY
#error "Please define your platforms home directory as APP_HOME_DIRECTORY."
#endif

ErrorType Storage::init() {
    ErrorType error = ErrorType::Success;

    if (!_status.isInitialized) {
        std::array<char, 32> mediumString;

        switch (medium()) {
            case StorageTypes::Medium::Flash:
                strncpy(mediumString.data(), "abstractionLayerFlashStorage", sizeof(mediumString));
                break;
            case StorageTypes::Medium::Eeprom:
                strncpy(mediumString.data(), "abstractionLayerEepromStorage", sizeof(mediumString));
                break;
            case StorageTypes::Medium::Sd:
                strncpy(mediumString.data(), "abstractionLayerSdStorage", sizeof(mediumString));
                break;
            case StorageTypes::Medium::Otp:
                strncpy(mediumString.data(), "abstractionLayerOtpStorage", sizeof(mediumString));
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        _rootPrefix.set(StaticString::Data<sizeof(APP_HOME_DIRECTORY "/") + mediumString.size()>(APP_HOME_DIRECTORY "/"));
        _rootPrefix->append(mediumString.data());
        mkdir(_rootPrefix->c_str(), S_IRWXU); 

        _status.isInitialized = true;
    }

    return error;
} 

ErrorType Storage::deinit() {
    return ErrorType::NotAvailable;
} 

ErrorType Storage::mainLoop() {
    return runNextEvent();
}