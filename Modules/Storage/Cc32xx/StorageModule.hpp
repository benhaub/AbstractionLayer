#ifndef __STORAGE_MODULE_HPP__
#define __STORAGE_MODULE_HPP__

//AbstractionLayer
#include "StorageAbstraction.hpp"
#include "Global.hpp"

class Storage : public StorageAbstraction, public Global<Storage> {
    public:
    Storage(StorageTypes::Medium medium) : StorageAbstraction(medium), Global<Storage>(){
        _status.isInitialized = false;
    }
    virtual ~Storage() = default;

    ErrorType init() override;
    ErrorType deinit() override;

    ErrorType mainLoop() override;
};

#endif //__STORAGE_MODULE_HPP__