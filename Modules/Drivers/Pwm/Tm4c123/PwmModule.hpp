#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

//AbstractionLayer
#include "PwmAbstraction.hpp"
//C++
#include <array>

class Pwm : public PwmAbstraction {
    public:
    Pwm() : PwmAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(Percent on) override;
    ErrorType setPeriod(const Microseconds period) override;

    private:
    /// @brief AbstractionLayer peripheral number to tm4c123 peripheral number
    uint32_t toTm4c123SysCtlPeripheralNumber(const PeripheralNumber peripheralNumber, ErrorType &error);
    /// @brief AbstractionLayer base register to tm4c123 peripheral number
    Register toTm4c123PeripheralBaseRegister(const PeripheralNumber peripheralNumber, ErrorType &error);
    /**
     * @brief Get an output from the next available generator
     * @param[out] error The error that occured while getting the next available output
     * @param[out] The generator from which the output is provided by.
     * @returns The next available output. Valid only if error is ErrorType::Success
     */
    uint32_t getOutputFromNextAvailableGenerator(ErrorType &error, uint32_t &generator);
    /**
     * @brief Return an output to the available outputs
     * @param[in] output The output to return
     * @returns ErrorType::Success
     */
    ErrorType returnOutputToGenerator(const uint32_t output);
    /// @brief Get the output bits from a generator output
    uint32_t toOutputBits(const uint32_t generatorOutput, ErrorType &error);

    /// @brief  The generator from which the output is provided by.
    uint32_t _generator;
    /// @brief  The output from the generator.
    uint32_t _output;
    /// @brief Whether the module is initialized or not.
    bool _isInitialized = false;
};

#endif // __PWM_MODULE_HPP__