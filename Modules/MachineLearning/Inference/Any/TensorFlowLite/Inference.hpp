#ifndef __INFERENCE_HPP__
#define __INFERENCE_HPP__

//AbstractionLayer
#include "InferenceAbstraction.hpp"
//TensorFlow Lite
#include "micro_interpreter.h"

/**
 * @class TensorFlowLiteInference
 * @brief TensorFlow Lite implementation of the InferenceAbstraction interface.
 */
class MachineLearningInference : public InferenceAbstraction {

    public:
    /// @brief Default destructor
    ~MachineLearningInference() override = default;

    ErrorType init() override;
    ErrorType loadModel(std::string_view modelData) override;
    ErrorType setInput(std::string_view inputData, const Count inputNumber) override;
    ErrorType runInference() override;
    ErrorType getOutput(StaticString::Container &outputData, const Count outputIndex) override;
    ErrorType getOutput(std::string &outputData, const Count outputIndex) override;
};

#endif // __INFERENCE_HPP__