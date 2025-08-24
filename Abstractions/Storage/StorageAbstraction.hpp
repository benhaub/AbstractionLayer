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
#include "Log.hpp"
#include "StaticString.hpp"

/**
 * @namespace StorageTypes
 * @brief Types related to storage
 */
namespace StorageTypes {

    /**
     * @struct Status
     * @brief The status of the storage.
    */
    struct Status {
        bool isInitialized; ///< Is the storage initialized and ready to use.
    };

    /**
     * @enum Medium
     * @brief The type of storage medium being used.
     */
    enum class Medium : uint8_t {
        Unknown = 0, ///< Unknown
        Flash,       ///< Flash
        Eeprom,      ///< Electrically Eraseable Programmable Memory
        Sd,          ///< Secure Digital
        Otp          ///< One Time Programmable
    };
}

/**
 * @class StorageAbstraction
 * @brief Interface for any storage type
*/
class StorageAbstraction : public EventQueue {

    public:
    /**
     * @brief Constructor
     * @param[in] medium The storage medium to use
     * @note For various storage mediums, the recommendation is to create classes for each medium within your modules and use switch cases
     *       in the StorageModule to dispatch the correct implementation
    */
    StorageAbstraction(StorageTypes::Medium medium) : EventQueue(), _medium(medium) {}
    /// @brief Destructor
    virtual ~StorageAbstraction() = default;


    /// @brief Tag for logging
    static constexpr char TAG[] = "Storage";

    /// @brief Print the status of the storage
    void printStatus() {
        PLT_LOGI(TAG, "<StorageStatus> <Initialized:%s> <Pie>",
            status().isInitialized ? "true" : "false");
    }

    /**
     * @brief Initializes the storage
     * @returns ErrorType::Success if the storage was initialized
     * @returns ErrorType::Failure otherwise.
    */
    virtual ErrorType init() = 0;
    /**
     * @brief Deinitializes the storage
     * @returns ErrorType::Success if the storage was deinitialized
     * @returns ErrorType::Failure otherwise.
    */
    virtual ErrorType deinit() = 0;

    /// @brief Get the status of the storage as a constant reference
    const StorageTypes::Status &status() { return _status; }
    /// @brief Get the storage medium
    StorageTypes::Medium medium() const { return _medium; }
    /// @brief The prefix of storage where filesystems will be mounted as a constant reference.
    const StaticString::Container &rootPrefix() const { return _rootPrefix; }

    private:
    /// @brief The storage medium.
    StorageTypes::Medium _medium;

    protected:
    /// @brief The status of the storage.
    StorageTypes::Status _status = { .isInitialized = false };
    /// @brief Typically for desktop systems. Specifies the location which to emulate the storage.
    StaticString::Container _rootPrefix;
};

#endif //__STORAGE_ABSTRACTION_HPP__
