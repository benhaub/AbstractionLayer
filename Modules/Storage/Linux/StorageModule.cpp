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
    ErrorType error = ErrorType::NotAvailable;

    if (!_status.isInitialized) {

        _rootPrefix.set(StaticString::Data<StorageTypes::longestMediumStringSize() + sizeof(APP_HOME_DIRECTORY "/")>());
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