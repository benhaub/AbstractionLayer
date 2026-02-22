/***************************************************************************//**
* @author  Ben Haubrich
* @file    InferenceAbstraction.hpp
* @details Interface for machine learning model inference.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __INFERENCE_ABSTRACTION_HPP__
#define __INFERENCE_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
#include "StaticString.hpp"
//C++
#include <string_view>

/**
 * @namespace InferenceAbstractionTypes
 * @brief Namespace for types used in the InferenceAbstraction interface.
 */
namespace InferenceAbstractionTypes {
    /**
     * @enum ModelFeatures
     * @brief Features that the model needs to use for inference
     * @details The model will need all of the layers implemented that you used to build while training the model.
     * @sa netron.app
     */
    enum class ModelFeatures : uint8_t {
        Unknown                          = 0,  ///< Unknown model feature
        DenseLayers                      = 1,  ///< Model uses dense (fully connected) layers
        Convolutional2dLayers            = 2,  ///< Model uses 2D convolutional layers
        DepthwiseConv2dLayers            = 3,  ///< Model uses depthwise 2D convolutional layers
        ReluActivation                   = 4,  ///< Model uses ReLU activation functions
        Relu6Activation                  = 5,  ///< Model uses ReLU6 activation functions
        SigmoidActivation                = 6,  ///< Model uses Sigmoid activation functions
        TanhActivation                   = 7,  ///< Model uses Tanh activation functions
        SoftMaxActivation                = 8,  ///< Model uses SoftMax activation functions
        MaxPooling2dSpatialDimension     = 9,  ///< Model uses 2D max pooling
        AveragePooling2dSpatialDimension = 10, ///< Model uses 2D average pooling
        AddElementWise                   = 11, ///< Model uses element-wise addition
        SubElementWise                   = 12, ///< Model uses element-wise subtraction
        MulElementWise                   = 13, ///< Model uses element-wise multiplication
        DivElementWise                   = 14, ///< Model uses element-wise division
        SquareElementWise                = 15, ///< Model uses element-wise squaring
        SqrtElementWise                  = 16, ///< Model uses element-wise square root
        ExpElementWise                   = 17, ///< Model uses element-wise exponentiation
        LogElementWise                   = 18, ///< Model uses element-wise logarithm
        ReshapeManipulation              = 19, ///< Model uses tensor dimension reshaping
        SqueezeManipulation              = 20, ///< Model uses tensor dimension squeezing
        ExpandDimsManipulation           = 21, ///< Model uses tensor dimension expansion
        StrippedSlicingManipulation      = 22, ///< Model uses tensor dimension slicing
        TransposeManipulation            = 23, ///< Model uses tensor dimension transposition
        BilinearResizeManipulation       = 24, ///< Model uses tensor dimension bilinear resizing
        NearestNeighborResizeManipulation= 25, ///< Model uses tensor dimension nearest neighbor resizing
        MeanManipulation                 = 26, ///< Model uses tensor dimension averaging
        ConcatenationDataFlow            = 27, ///< Model uses tensor concatenation
        SliceDataFlow                    = 28, ///< Model uses tensor slicing
        PadDataFlow                      = 29, ///< Model uses tensor padding
        UnpackDataFlow                   = 30, ///< Model uses tensor unpacking
        SvdfSequencing                   = 31, ///< Model uses SVDF layers for sequential data
        UnidirectionalSequencing         = 32, ///< Model uses unidirectional layers for sequential data
        LstmSequencing                   = 33, ///< Model uses LSTM layers for sequential data
        QuanitizationConversion          = 34, ///< Model uses quantization for data type conversion
        DequantizationConversion         = 35, ///< Model uses dequantization for data type conversion
        GatherConversion                 = 36  ///< Model uses gather for data type conversion
    };
}

/**
 * @class InferenceAbstraction
 * @brief Interface for machine learning model inference.
 */
class InferenceAbstraction {

    public:
    /// @brief Default destructor
    virtual ~InferenceAbstraction() = default;

    /**
     * @brief Initialize the inference model.
     * @returns ErrorType::Success if initialized successfully.
     * @returns ErrorType::Failure if not initialized.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Load a trained model to be used for inference.
     * @param[in] modelData The pre-trained data model.
     * @returns ErrorType::Success if loaded.
     * @returns ErrorType::Failure not loaded.
     */
    virtual ErrorType loadModel(std::string_view modelData) = 0;
    /**
     * @brief Provide an input tensor to the model for inference.
     * @param[in] inputData The input tensor data.
     * @param[in] inputIndex If the model has multiple inputs, the index of the input to set. If there is one input then use 0.
     * @returns ErrorType::Success if input was given to the model
     * @returns ErrorType::InvalidParameter if the input data is not the correct size
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType setInput(std::string_view inputData, const Count inputIndex) = 0;
    /**
     * @brief Run inference on the loaded model with the provided input tensor.
     * @returns ErrorType::Success if inference was run successfully
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType runInference() = 0;
    /**
     * @brief Read the prediction results from the model after inference.
     * @param[out] outputData The output tensor data.
     * @param[in] outputIndex If the model has multiple outputs, the index of the output to read. If there is one output then use 0.
     * @returns ErrorType::Success if output was read from the model
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType getOutput(StaticString::Container &outputData, const Count outputIndex) = 0;
    /// @copydoc getOutput(StaticString::Container &outputData)
    virtual ErrorType getOutput(std::string &outputData, const Count outputIndex) = 0;
};

#endif // __INFERENCE_ABSTRACTION_HPP__