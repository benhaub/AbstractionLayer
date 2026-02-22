//AbstractionLayer
#include "Inference.hpp"
//Tensflow Lite
#include "micro_interpreter.h"
#include "micro_log.h"
#include "micro_mutable_op_resolver.h"
#include "micro_profiler.h"
#include "system_setup.h"
#include "schema_generated.h"

#ifndef APP_TFLITE_MODEL_FEATURES
#error "TFLITE_MODEL_FEATURES is not defined.\
        Please define using InferenceAbstractionTypes::ModelFeatures as a comma separated list."
#endif

#ifndef APP_TFLITE_ALLOCATED_MEMORY_SIZE_BYTES
#error "Define APP_TFLITE_ALLOCATED_MEMORY_SIZE_BYTES.\
        Use https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/tools/benchmarking\
        to figure out what size is appropriate for your model"
#endif

namespace {
    /// @brief Initialize the opResolver
    template <InferenceAbstractionTypes::ModelFeatures... Features>
    inline tflite::MicroMutableOpResolver<sizeof...(Features)> &OpResolver() {
        constexpr size_t numFeatures = sizeof...(Features);
        static tflite::MicroMutableOpResolver<numFeatures> opResolver;

        return opResolver;
    }

    /// @brief Add the feature to the model.
    /// @details The output of tflm_benchmark will show you which features your model uses.
    /// @tparam _Feature the feature to add
    /// @sa https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/tools/benchmarking/README.md
    template <InferenceAbstractionTypes::ModelFeatures _Feature>
    inline constexpr void AddFeature(auto &opResolver) {
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::DenseLayers) {opResolver.AddFullyConnected();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::Convolutional2dLayers) {opResolver.AddConv2D();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::ReluActivation) {opResolver.AddRelu();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::Relu6Activation) {opResolver.AddRelu6();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::SoftMaxActivation) {opResolver.AddSoftmax();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::ReshapeManipulation) {opResolver.AddReshape();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::QuanitizationConversion) {opResolver.AddQuantize();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::MulElementWise) {opResolver.AddMul();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::BilinearResizeManipulation) {opResolver.AddResizeBilinear();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::MaxPooling2dSpatialDimension) {opResolver.AddMaxPool2D();}
        if constexpr (_Feature == InferenceAbstractionTypes::ModelFeatures::MeanManipulation) {opResolver.AddMean();}
    }

    /// @brief Add features to the model
    /// @tparam Features the features to add.
    template <InferenceAbstractionTypes::ModelFeatures... Features>
    inline void AddFeatures() {
        (AddFeature<Features>(OpResolver<APP_TFLITE_MODEL_FEATURES>()), ...);
    }

    std::array<uint8_t, APP_TFLITE_ALLOCATED_MEMORY_SIZE_BYTES> tensorArena{};

    /**
     * @brief Create the interpreter
     * @details The interpreter can not be moved or copied, so it is static.
     * @param modelData the model data
     * @return the interpreter
     */
    inline tflite::MicroInterpreter &Interpreter(std::string_view modelData = std::string_view()) {
        static tflite::MicroInterpreter interpreter(tflite::GetModel(modelData.data()),
                                                    OpResolver<APP_TFLITE_MODEL_FEATURES>(),
                                                    &tensorArena[0],
                                                    APP_TFLITE_ALLOCATED_MEMORY_SIZE_BYTES);

        return interpreter;
    }
}

ErrorType MachineLearningInference::init() {
    AddFeatures<APP_TFLITE_MODEL_FEATURES>();

    return ErrorType::Success;
}

ErrorType MachineLearningInference::loadModel(std::string_view modelData) {
    ErrorType error = ErrorType::Failure;

    tflite::MicroInterpreter &interpreter = Interpreter(modelData);
    kTfLiteOk == interpreter.AllocateTensors() ? error = ErrorType::Success : error = ErrorType::Failure;

    return error;
}

ErrorType MachineLearningInference::setInput(std::string_view inputData, const Count inputNumber) {
    ErrorType error = ErrorType::InvalidParameter;
    size_t requiredInputShape = 1;

    //TODO: Assume all inputs are the same size?
    if (Interpreter().input(inputNumber) != nullptr) {

        requiredInputShape = Interpreter().input(inputNumber)->bytes;

        if (requiredInputShape == inputData.size()) {
            std::copy_n(&inputData[0], inputData.size(), Interpreter().input(inputNumber)->data.raw);
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType MachineLearningInference::runInference() {
    return kTfLiteOk == Interpreter().Invoke() ? ErrorType::Success : ErrorType::Failure;
}

template <typename StringLike>
requires std::is_same_v<StringLike, std::string *> || std::is_same_v<StringLike, StaticString::Container &>
inline ErrorType getOutputImplementation(StringLike &&outputData, const Count outputIndex) {
    outputData->assign(std::string_view(&(Interpreter().output(outputIndex)->data.raw_const[0]), Interpreter().output(outputIndex)->bytes));

    return ErrorType::Success;
}
ErrorType MachineLearningInference::getOutput(StaticString::Container &outputData, const Count outputIndex) {
    return getOutputImplementation(outputData, outputIndex);
}
ErrorType MachineLearningInference::getOutput(std::string &outputData, const Count outputIndex) {
    return getOutputImplementation(&outputData, outputIndex);
}