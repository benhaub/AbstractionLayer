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

    virtual ErrorType setCaptureMode(GptmCaptureTypes::Mode mode) = 0;
    virtual ErrorType setPeriod(Milliseconds period) = 0;

};

#endif //__GPTM_CAPTURE_ABSTRACTION_HPP__