#ifndef __STORAGE_MODULE_HPP__
#define __STORAGE_MODULE_HPP__

//AbstractionLayer
#include "Global.hpp"
#include "StorageAbstraction.hpp"

class Storage final : public StorageAbstraction, public Global<Storage, StorageTypes::Medium> {

    public:
    Storage(StorageTypes::Medium medium) : StorageAbstraction(medium), Global<Storage, StorageTypes::Medium>() {
        _status.isInitialized = false;
    }

    ErrorType init() override;
    ErrorType deinit() override;

    ErrorType mainLoop() override;

    private:
    ErrorType getEnvironment(std::string variable, ErrorType &error, std::array<char, 32> &expandedVariable);
};

#endif //__STORAGE_MODULE_HPP__