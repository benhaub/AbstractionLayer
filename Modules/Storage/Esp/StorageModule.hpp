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
    /**
     * @brief Constructor
     * @param[in] name The name of the storage
     * @param[in] os The operating system
     * @attention Stdlib version is a Global class that is constructed with Init()
    */
    Storage(StorageTypes::Medium medium) : StorageAbstraction(medium) {
        _status.isInitialized = false;
    }
    virtual ~Storage() = default;

    ErrorType init() override;
    ErrorType deinit() override;

    ErrorType mainLoop() override;
};

#endif //__CBT_FLASH_HPP__