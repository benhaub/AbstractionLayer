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
    constexpr auto longestMediumStringSize = []() -> size_t {
        size_t longestSoFar = 0;

        for (const auto &mapPair : StorageTypes::MediumToStringPairs) {
            if (mapPair.second.size() > longestSoFar) {
                longestSoFar = mapPair.second.size();
            }
        }

        return longestSoFar;
    };

    ErrorType error = ErrorType::NotAvailable;

    if (!_status.isInitialized) {

        _rootPrefix.set(StaticString::Data<longestMediumStringSize() + sizeof(APP_HOME_DIRECTORY "/")>());
        std::string_view mediumString = StorageTypes::MediumToString(medium());

        if (0 != mediumString.compare(StorageTypes::MediumToString(StorageTypes::Medium::Unknown))) {
            _rootPrefix->assign(APP_HOME_DIRECTORY "/");
            _rootPrefix->append(mediumString);
            mkdir(_rootPrefix->c_str(), S_IRWXU);

            _status.isInitialized = true;
            error = ErrorType::Success;
        }
    }

    return error;
} 

ErrorType Storage::deinit() {
    return ErrorType::NotAvailable;
} 

ErrorType Storage::mainLoop() {
    return runNextEvent();
}