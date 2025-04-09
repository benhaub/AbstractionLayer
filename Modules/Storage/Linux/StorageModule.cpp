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

ErrorType Storage::init() {
    if (_status.isInitialized) {
        return ErrorType::Success;
    }

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
            return ErrorType::InvalidParameter;
    }

    ErrorType error;
    constexpr char home[] = "HOME";
    std::array<char, 32> expandedHome;
    getEnvironment(home, error, expandedHome);
    _rootPrefix.assign(expandedHome.data(), strlen(expandedHome.data()));
    _rootPrefix.append("/");
    _rootPrefix.append(mediumString.data(), strlen(mediumString.data()));
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

ErrorType Storage::getEnvironment(std::string variable, ErrorType &error, std::array<char, 32> &expandedVariable) {
    
    const char *environmentVariable = std::getenv(variable.data());
    if (nullptr == environmentVariable) {
        error = ErrorType::Failure;
    }
    else {
        strncpy(expandedVariable.data(), environmentVariable, expandedVariable.size());
        error = ErrorType::Success;
    }

    return error;
}