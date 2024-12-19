#include "GptmCaptureModule.hpp"

ErrorType GptmCaptureModule::init() {
    return ErrorType::NotImplemented;
}

ErrorType GptmCaptureModule::setHardwareConfig(PeripheralIdentifier identifier, void *peripheral) {
    return ErrorType::NotImplemented;
}

ErrorType GptmCaptureModule::deinit() {
    return ErrorType::NotImplemented;
}

ErrorType GptmCaptureModule::start() { 
    return ErrorType::NotImplemented;
}

ErrorType GptmCaptureModule::stop() {
    return ErrorType::NotImplemented;
}

ErrorType GptmCaptureModule::setCaptureMode(GptmCaptureTypes::Mode mode) {
    return ErrorType::NotImplemented;
}

ErrorType GptmCaptureModule::setPeriod(Milliseconds period) {
    return ErrorType::NotImplemented;
}
ErrorType GptmCaptureModule::readCaptureValue(Count &captureValue) {
    return ErrorType::NotImplemented;
}