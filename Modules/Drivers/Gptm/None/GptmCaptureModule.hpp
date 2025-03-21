#ifndef __GPTM_CAPTURE_MODULE_HPP__
#define __GPTM_CAPTURE_MODULE_HPP__

#include "GptmCaptureAbstraction.hpp"

class GptmCaptureModule : public GptmCaptureAbstraction {

    public:
    GptmCaptureModule() : GptmCaptureAbstraction() {}
    ~GptmCaptureModule() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;

    ErrorType setCaptureMode(GptmCaptureTypes::Mode mode) override;
    ErrorType readCaptureValue(Count &captureValue) override;
};

#endif //__GPTM_CAPTURE_MODULE_HPP__