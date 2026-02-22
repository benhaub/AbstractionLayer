#include "Inference.hpp"

ErrorType MachineLearningInference::init() {
    return ErrorType::NotImplemented;
}

ErrorType MachineLearningInference::loadModel(std::string_view modelData) {
    return ErrorType::NotImplemented;
}

ErrorType MachineLearningInference::setInput(std::string_view inputData, const Count inputIndex) {
    return ErrorType::NotImplemented;
}

ErrorType MachineLearningInference::runInference() {
    return ErrorType::NotImplemented;
}

ErrorType MachineLearningInference::getOutput(StaticString::Container &outputData, const Count outputIndex) {
    return ErrorType::NotImplemented;
}

ErrorType MachineLearningInference::getOutput(std::string &outputData, const Count outputIndex) {
    return ErrorType::NotImplemented;
}