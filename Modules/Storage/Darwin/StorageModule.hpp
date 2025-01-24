#ifndef __STORAGE_MODULE_HPP__
#define __STORAGE_MODULE_HPP__

//AbstractionLayer
#include "Global.hpp"
#include "StorageAbstraction.hpp"

/**
 * @class Storage
*/
class Storage : public StorageAbstraction, public Global<Storage, std::string> {

    public:
    Storage(std::string name, StorageTypes::Medium medium) : StorageAbstraction(name, medium), Global<Storage, std::string>() {
        _status.isInitialized = false;
    }
    virtual ~Storage() = default;

    ErrorType initStorage() override;
    ErrorType deinitStorage() override;
    ErrorType maxStorageSize(Kilobytes &size, std::string partitionName = std::string()) override;
    ErrorType availableStorage(Kilobytes &size, std::string partitionName = std::string()) override;
    ErrorType maxRamSize(Kilobytes &size, std::string memoryRegionName = std::string()) override;
    ErrorType availableRam(Kilobytes &size, std::string memoryRegionName = std::string()) override;
    ErrorType erasePartition(const std::string &partitionName = std::string()) override;
    ErrorType eraseAllPartitions() override;

    ErrorType mainLoop() override;

    private:
    ErrorType deinitStorageInternal();
    ErrorType maxStorageSizeInternal(Bytes &size, std::string partitionName);
    ErrorType availableStorageInternal(Bytes &size, std::string partitionName);
    ErrorType erasePartitionInternal(const std::string &partitionName);
    ErrorType eraseAllPartitionsInternal();

    std::string getEnvironment(std::string variable, ErrorType &error);
};

#endif //__STORAGE_MODULE_HPP__