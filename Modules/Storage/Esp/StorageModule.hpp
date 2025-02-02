#ifndef __STORAGE_MODULE_HPP__
#define __STORAGE_MODULE_HPP__

//AbstractionLayer
#include "Global.hpp"
#include "StorageAbstraction.hpp"
//ESP
#include "nvs_handle.hpp"

/**
 * @class Storage
*/
class Storage : public StorageAbstraction, public Global<Storage, std::string> {

    public:
    /**
     * @brief Constructor
     * @param[in] name The name of the storage
     * @param[in] os The operating system
     * @attention Stdlib version is a Global class that is constructed with Init()
    */
    Storage(std::string name, StorageTypes::Medium medium) : StorageAbstraction(name, medium) {
        _status.isInitialized = false;
    }
    virtual ~Storage() = default;

    ErrorType initStorage() override;
    ErrorType deinitStorage() override;
    ErrorType maxStorageSize(Kilobytes &size, std::string partitionName = std::string()) override;
    ErrorType availableStorage(Kilobytes &size, std::string partitionName = std::string()) override;
    ErrorType erasePartition(const std::string &partitionName) override;
    ErrorType eraseAllPartitions() override;

    ErrorType mainLoop() override;

    private:
    std::unique_ptr<nvs::NVSHandle> _nvsHandle;
};

#endif //__CBT_FLASH_HPP__