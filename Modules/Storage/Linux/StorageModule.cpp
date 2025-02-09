//AbstractionLayer
#include "Error.hpp"
#include "StorageModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Posix
#include <sys/statvfs.h>
#include <sys/stat.h>

ErrorType Storage::init() {
    if (_status.isInitialized) {
        return ErrorType::Success;
    }

    std::string mediumString;
    switch (medium()) {
        case StorageTypes::Medium::Flash:
            mediumString = "abstractionLayerFlashStorage";
            break;
        case StorageTypes::Medium::Eeprom:
            mediumString = "abstractionLayerEepromStorage";
            break;
        case StorageTypes::Medium::Sd:
            mediumString = "abstractionLayerSdStorage";
            break;
        case StorageTypes::Medium::Otp:
            mediumString = "abstractionLayerOtpStorage";
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    ErrorType error;
    _rootPrefix = getEnvironment("HOME", error);
    _rootPrefix.append("/").append(mediumString);
    mkdir(_rootPrefix.c_str(), S_IRWXU); 

    _status.isInitialized = true;
    return error;
} 

ErrorType Storage::deinit() {
    return ErrorType::NotAvailable;
} 

ErrorType Storage::mainLoop() {
    return runNextEvent();
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