/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   StorageAbstraction.hpp
* @details \b Synopsis: \n Interface for any storage type
* @ingroup Abstractions
******************************************************************************/
#ifndef __STORAGE_ABSTRACTION_HPP__
#define __STORAGE_ABSTRACTION_HPP__

//AbstractionLayer
#include "EventQueue.hpp"

/**
 * @struct StorageStatus
 * @brief The status of the storage.
*/
struct StorageStatus {
    bool isInitialized; ///< Is the storage initialized and ready to use.
};

/**
 * @class StorageAbstraction
 * @brief Foundation core functions for any storage type
*/
class StorageAbstraction : public EventQueue {

    public:
    /**
     * @brief Constructor
     * @param [in] name The name of the storage
    */
    StorageAbstraction(std::string name) : EventQueue(), _name(name) {}
    /// @brief Destructor
    virtual ~StorageAbstraction() = default;

    /**
     * @brief Initializes the storage
     * @returns ErrorType::Success if initiatized.\n
     * @returns ErrorType::Failure if failed.\n
     * @returns ErrorType::NotImplemented if not be implemented.\n
     * @returns ErrorType::NotAvailable if the storage does not need to be initialized.\n
    */
    virtual ErrorType initStorage() = 0;
    /**
     * @brief Deinitializes the storage
     * @returns ErrorType::Success if the storage was deinitialized.
     * @returns ErrorType::Failure if it was not deinitialized.
    */
    virtual ErrorType deinitStorage() = 0;
    /**
     * @brief Get the size of the storage
     * @param[out] size The size of the storage
     * @param[in] partitionName The name of the partition to get the max storage size from. Default partition name used if not provided.
     * @pre initStorage must be called first
     * @returns ErrorType::Success if the operation is successful, otherwise an error code
     * @returns ErrorType::NotImplemented if not be implemented
     * @returns ErrorType::Failure for any other failure that prevents the operation from returning the max storage size to size
     */
    virtual ErrorType maxStorageSize(Kilobytes &size, std::string partitionName = std::string()) = 0;
    /**
     * @brief Get the size of the storage
     * @param[out] size The size of the storage
     * @param[in] partitionName The name of the partition to get the max storage size from. Default partition name used if not provided.
     * @pre initStorage must be called first
     * @returns ErrorType::Success if the operation is successful, otherwise an error code
     * @returns ErrorType::PrerequisiteNotMet if initStorage has not been called
     * @returns ErrorType::NotImplemented if not implemented
     * @returns ErrorType::Failure for any other failure that prevents the operation from returning the available storage size to size
     */
    virtual ErrorType availableStorage(Kilobytes &size, std::string partitionName = std::string()) = 0;
    /**
     * @brief Get the size of RAM
     * @param[out] size The size of the RAM
     * @param[in] memoryRegionName The name of the RAM memory region to get the size of
     * @pre initStorage must be called first.
     * @returns ErrorType::Success if the size of the ram was returned.
     * @returns ErrorType::InvalidParameter if the memoryRegionName does not exist.
     * @returns ErrprType::Failure if the size of the ram could not be returned for any other reason.
     */
    virtual ErrorType maxRamSize(Kilobytes &size, std::string memoryRegionName = std::string()) = 0;
    /**
     * @brief Get the amount of RAM used
     * @details avaialableRam will never show the true available ram because it's not possible to track every stack push.
     *          It's easiest to account for static ram usage taken up by .bss and .data sections, and then also the amount of
     *          heap memory currently being used. It should also be possible to track single large stack allocations like
     *          creating a stack for a thread or a mempool.
     * @param[out] size The amount of ram used.
     * @param[in] memoryRegionName The name of the RAM memory region to get the used size of.
     * @returns ErrorType::Success if the size of the ram was returned.
     * @returns ErrorType::InvalidParameter if the memoryRegionName does not exist.
     * @returns ErrprType::Failure if the size of the ram could not be returned for any other reason.
     */
    virtual ErrorType availableRam(Kilobytes &size, std::string memoryRegionName = std::string()) = 0;
    /**
     * @brief erase the specified partition
     * @param[in] partitionName The name of the partition to erase
     * @returns ErrorType::Success if the operation is successful.
     * @returns ErrorType::Failure if the operation fails.
    */
    virtual ErrorType erasePartition(const std::string &partitionName) = 0;
    /**
     * @brief erase all partitions
     * @returns ErrorType::Success if the operation is successful.
     * @returns ErrorType::Failure if the operation fails.
    */
    virtual ErrorType eraseAllPartitions() = 0;

    /// @brief Get the status of the storage as a mutable reference
    StorageStatus &status() { return _status; }
    /// @brief Get the status of the storage as a constant reference
    const StorageStatus &statusConst() const { return _status; }
    /// @brief Get the name of the storage
    const std::string &name() const { return _name; }
    /// @brief Get the root prefix
    const std::string &rootPrefix() const { return _rootPrefix; }

    /// @brief Tag for logging
    static constexpr char TAG[] = "Storage";

    private:
    /// @brief The name of the storage.
    std::string _name;

    protected:
    /// @brief The status of the storage.
    StorageStatus _status = { .isInitialized = false };
    /// @brief A path that is prepened to all file names for storage
    std::string _rootPrefix;
};

#endif //__STORAGE_ABSTRACTION_HPP__