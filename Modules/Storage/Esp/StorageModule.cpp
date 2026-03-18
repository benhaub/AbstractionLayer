//AbstractionLayer
#include "StorageModule.hpp"

ErrorType Storage::init() {
    _status.isInitialized = true;
    return ErrorType::NotAvailable;
} 

ErrorType Storage::deinit() {
    _status.isInitialized = false;
    return ErrorType::NotAvailable;
}