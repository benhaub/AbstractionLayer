/***************************************************************************//**
* @author   Ben Haubrich
* @file     Gptm/CaptureAbstraction.hpp
* @details  Abstraction layer for GPTM Capture
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __GPTM_CAPTURE_ABSTRACTION_HPP__
#define __GPTM_CAPTURE_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
#include "GptmAbstraction.hpp"
//C++
#include <functional>

namespace GptmCaptureTypes {

    /**
     * @enum Mode
     * @brief The capture mode for the timer capture
     */
    enum class Mode : uint8_t {
        Unknown = 0,
        FallingEdge,
        RisingEdge,
        AnyEdge
    };
}

/**
 * @class GptmCaptureAbstraction
 * @brief Abstraction layer for GPTM Capture
*/
class GptmCaptureAbstraction : public GptmAbstraction {

    public:
    ///@brief Constructor
    GptmCaptureAbstraction() : GptmAbstraction() {}
    ///@brief Destructor
    virtual ~GptmCaptureAbstraction() = default;

    /**
     * @brief Set when the capture should occur on the input signal
     * @returns ErrorType::Success if the mode was set
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType setCaptureMode(GptmCaptureTypes::Mode mode) = 0;
    /**
     * @brief Set the period of the capture
     * @returns ErrorType::Success if the period was set
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType setPeriod(Milliseconds period) = 0;
    /**
     * @brief Read the capture value
     * @param[out] captureValue The value of the capture
     * @returns ErrorType::Success if the value was read
     * @returns ErrorType::NoData if there is no data to read
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType readCaptureValue(Count &captureValue) = 0;
};

#endif //__GPTM_CAPTURE_ABSTRACTION_HPP__