/***************************************************************************//**
* @author   Ben Haubrich
* @file     GptmAbstraction.hpp
* @details  Abstraction layer for GPTM
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __GPTM_ABSTRACTION_HPP__
#define __GPTM_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"

/**
 * @class GptmAbstraction
 * @brief Abstraction layer for GPTM.
*/
class GptmAbstraction {

    public:
    ///@brief Constructor
    GptmAbstraction() = default;
    ///@brief Destructor
    virtual ~GptmAbstraction() = default;

    static constexpr char TAG[] = "Gptm";

    /**
     * @brief Initialize the counter
     * @pre Call setHardwareConfig first.
     * @returns ErrorType::Success if the counter was initialized.
     * @returns ErrorType::Failure otherse.
     * @post The counter is ready to use.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Set the hardware configuration for the counter
     * @param[in] identifierType The type of the peripheral
     * @sa PeripheralIdentifier
     * @param[in] peripheral Either the base register of the periperhal to use or the numbered index.
     * @returns ErrorType::Success if the hardware was configured
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType setHardwareConfig(PeripheralIdentifier identifierType, void *peripheral) = 0;
    /**
     * @brief deinitialize the counter
     * @returns ErrorType::Success if the counter was deinitialized
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType deinit() = 0;
    /**
     * @brief start counting. The way in which the counter counts depends on the type of counter used.
     * @returns ErrorType::Success if the counter was started
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType start() = 0;
    /**
     * @brief Stop counter.
     * @returns ErrorType::Success if the counter was stopped
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType stop() = 0;

    ///@brief Get a mutable reference to the peripheral number
    PeripheralNumber &peripheralNumber() { return _peripheralNumber; }
    ///@brief Get a constant reference to the peripheral number
    const PeripheralNumber &peripheralNumberConst() const { return _peripheralNumber; }
    ///@brief Get a mutable reference to the base pointer of the peripheral
    uint32_t &peripheralBasePointer() { return *_peripheralBasePointer; }
    ///@brief Get a constant reference to the base pointer of the peripheral
    const uint32_t &peripheralBasePointerConst() const { return *_peripheralBasePointer; }

    protected:
    ///@brief The peripheral number.
    PeripheralNumber _peripheralNumber = PeripheralNumber::Unknown;
    /// @brief The base pointer to the peripheral.
    uint32_t *_peripheralBasePointer = nullptr;
};

#endif //__GPTM_ABSTRACTION_HPP__