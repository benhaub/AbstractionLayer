#ifndef __GPTM_CAPTURE_MODULE_HPP__
#define __GPTM_CAPTURE_MODULE_HPP__

//AbstractionLayer
#include "GptmCaptureAbstraction.hpp"
//TI drivers
#include "ti/drivers/Capture.h"
//TI sysconfig (auto-generated)
#include "ti_drivers_config.h"

class GptmCaptureModule final : public GptmCaptureAbstraction {

    public:
    GptmCaptureModule() : GptmCaptureAbstraction() {
        Capture_Params_init(&_captureParams);
    }

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;

    ErrorType setCaptureMode(const GptmCaptureTypes::Mode mode) override;
    ErrorType readCaptureValue(Count &captureValue) override;

    private:
    Capture_Params _captureParams;
    Capture_Handle _captureHandle;

    uint_least8_t toTiDriverIndex(PeripheralNumber peripheralNumber, ErrorType &error) {
        error = ErrorType::Success;

        uint_least8_t index = static_cast<uint_least8_t>(peripheralNumber);

#ifdef CONFIG_TI_DRIVERS_CAPTURE_COUNT
        if (index > CONFIG_TI_DRIVERS_CAPTURE_COUNT) {
            error = ErrorType::InvalidParameter;
            index = CONFIG_TI_DRIVERS_CAPTURE_COUNT + 1;
        }
#endif

        return index;
    }
};

#endif //__GPTM_CAPTURE_MODULE_HPP__