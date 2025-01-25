#ifndef __STORAGE_MODULE_HPP__
#define __STORAGE_MODULE_HPP__

//AbstractionLayer
#include "StorageAbstraction.hpp"
#include "Global.hpp"

class Storage : public StorageAbstraction, public Global<Storage, std::string> {
    public:
    Storage(std::string name, StorageTypes::Medium medium) : StorageAbstraction(name, medium), Global<Storage, std::string>(){
        _status.isInitialized = false;
    }
    virtual ~Storage() = default;

    ErrorType initStorage() override;
    ErrorType deinitStorage() override;
    ErrorType maxStorageSize(Kilobytes &size, std::string partitionName) override;
    ErrorType availableStorage(Kilobytes &size, std::string partitionName) override;
    ErrorType erasePartition(const std::string &partitionName) override;
    ErrorType eraseAllPartitions() override;

    ErrorType mainLoop() override;
};

#endif //__STORAGE_MODULE_HPP__