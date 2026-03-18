//AbstractionLayer
#include "GptmCaptureModule.hpp"
#include "OperatingSystemModule.hpp"

static constexpr std::array<char, OperatingSystemTypes::MaxQueueNameLength> intervalQueueName = {"intervals"};

ErrorType GptmCaptureModule::init() {
    assert(PeripheralNumber::Unknown != peripheralNumber());
    constexpr int maxMessages = 10;

    ErrorType error = OperatingSystem::Instance().createQueue(intervalQueueName, sizeof(uint32_t), maxMessages);
    if (ErrorType::Success != error) {
        return error;
    }

    Capture_init();

    auto captureCallback = [](Capture_Handle handle, uint32_t interval, int_fast16_t status) -> void {
        const bool fromIsr = true;
        const bool toFront = false;
        OperatingSystem::Instance().sendToQueue(intervalQueueName, &interval, 0, toFront, fromIsr);
    };

    _captureParams.periodUnit = Capture_PERIOD_US;
    _captureParams.callbackFxn = captureCallback;

    _captureHandle = Capture_open(toTiDriverIndex(peripheralNumber(), error), &_captureParams);

    return ErrorType::Success;
}

ErrorType GptmCaptureModule::deinit() {
    Capture_close(_captureHandle);
    return ErrorType::Success;
}

ErrorType GptmCaptureModule::start() {
    Capture_start(_captureHandle);
    return ErrorType::Success;
}

ErrorType GptmCaptureModule::stop() {
    Capture_stop(_captureHandle);
    return ErrorType::Success;
}

ErrorType GptmCaptureModule::setCaptureMode(const GptmCaptureTypes::Mode mode) {
    ErrorType error = ErrorType::Success;

    switch (mode) {
        case GptmCaptureTypes::Mode::AnyEdge:
            _captureParams.mode = Capture_RISING_EDGE;
            break;
        case GptmCaptureTypes::Mode::FallingEdge:
            _captureParams.mode = Capture_FALLING_EDGE;
            break;
        case GptmCaptureTypes::Mode::RisingEdge:
            _captureParams.mode = Capture_RISING_EDGE;
            break;
        default:
            error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType GptmCaptureModule::readCaptureValue(Count &captureValue) {
    if (ErrorType::Success == OperatingSystem::Instance().receiveFromQueue(intervalQueueName, &captureValue, 0, false)) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::NoData;
    }
}