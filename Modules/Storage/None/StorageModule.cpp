//AbstractionLayer
#include "StorageModule.hpp"

ErrorType Storage::initStorage() {
    return ErrorType::NotImplemented;
} 
ErrorType Storage::deinitStorage() {
    return ErrorType::NotImplemented;
} 
ErrorType Storage::maxStorageSize(Bytes &size, std::string partitionName) {
    return ErrorType::NotImplemented;
}
ErrorType Storage::availableStorage(Bytes &size, std::string partitionName) {
    return ErrorType::NotImplemented;
}
ErrorType Storage::maxRamSize(Bytes &size, std::string memoryRegionName) {
    return ErrorType::NotImplemented;
}
ErrorType Storage::availableRam(Bytes &size, std::string memoryRegionName) {
    return ErrorType::NotImplemented;
}
ErrorType Storage::erasePartition(const std::string &partitionName) {
    return ErrorType::NotImplemented;
}
ErrorType Storage::eraseAllPartitions() {
    return ErrorType::NotImplemented;
}
ErrorType Storage::mainLoop() {
    return ErrorType::NotImplemented;
}