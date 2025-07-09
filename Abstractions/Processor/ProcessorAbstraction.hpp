/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   ProcessorAbstraction.hpp
* @details Interface for register access to the processor
* @ingroup Abstractions
*******************************************************************************/
#ifndef __PROCESSOR_ABSTRACTION_HPP__
#define __PROCESSOR_ABSTRACTION_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"

/**
 * @class ProcessorAbstraction
 * @brief Interface for register access to the processor
 */
class ProcessorAbstraction {
    public:
    ProcessorAbstraction() = default;
    virtual ~ProcessorAbstraction() = default;

    /**
     * @brief Checks if the current context is an interrupt context.
     * @returns ErrorType::Success if the current context is an interrupt context
     * @returns ErrorType::Negative if the current context is not an interrupt context
     * @returns ErrorType::NotAvailable If interrupt context can not be directly entered by the application (i.e. an operating system environment)
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType isInterruptContext() = 0;
    /**
     * @brief Get the Interrupt request number of the currently active interrupt
     * @param[out] interruptRequestNumber The Interrupt request number of the currently active interrupt
     * @returns ErrorType::Success if the interrupt request number was successfully retrieved
     * @returns ErrorType::Negative if the current context is not an interrupt context
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType interruptRequestNumber(uint32_t &interruptRequestNumber) = 0;
    /**
     * @brief Get the value of the stack pointer currently being used by the processor
     * @param[out] stackPointer The value of the stack pointer
     * @returns The value of the stack pointer currently being used by the processor
     */
    virtual ErrorType stackPointer(uint32_t &stackPointer) = 0;
};

#endif //__PROCESSOR_ABSTRACTION_HPP__
