#include "AdcModule.hpp"

ErrorType Adc::init() {
    if (PeripheralNumber::Unknown == params().peripheralNumber) {
        return ErrorType::PrerequisitesNotMet;
    }

    ADC_init();

    ADC_Params adcParameters;
    ADC_Params_init(&adcParameters);

    ErrorType error;
    _handle = ADC_open(toCc32xxAdcPeripheralNumber(params().peripheralNumber, error), &adcParameters);
    if (nullptr != _handle) {
        if (ErrorType::Success == error) {
            return ErrorType::Success;
        }
    }

    return ErrorType::Failure;
}

ErrorType Adc::deinit() {
    ADC_close(_handle);
    _handle = nullptr;
    return ErrorType::Success;
}

ErrorType Adc::convert(Count &rawValue) {
    uint16_t rawValue16 = 0;
    if (ADC_STATUS_SUCCESS != ADC_convert(_handle, &rawValue16)) {
        return ErrorType::Failure;
    }

    rawValue = rawValue16;

    return ErrorType::Success;
}

ErrorType Adc::rawToVolts(const Count rawValue, Volts &volts) {
#ifdef DeviceFamily_CC3220
    constexpr uint16_t compensationFactor = 2365;
#else
    constexpr uint16_t compensationFactor = 1000;
#endif

    if (rawValue > UINT16_MAX) {
        return ErrorType::InvalidParameter;
    }

    uint32_t microVolts = (compensationFactor * ADC_convertToMicroVolts(_handle, rawValue)) / 1000;
    volts = static_cast<float>(microVolts) / 1E6;

    return ErrorType::Success;
}