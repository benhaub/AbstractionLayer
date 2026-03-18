//AbstractionLayer
#include "AdcModule.hpp"

ErrorType Adc::init() {
    assert(params().peripheralNumber != PeripheralNumber::Unknown);

    const adc_oneshot_unit_init_cfg_t initConfig = {
        .unit_id = ADC_UNIT_1
    };

    esp_err_t err = adc_oneshot_new_unit(&initConfig, &adcHandle);
    if (ESP_OK == err) {
        const adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT
        };
        ErrorType error = ErrorType::Failure;
        //The pins whose alternate functions support the ADC and have analog functionality are exposed in a per-design basis.
        //Consult your PCB schematics to determine which pins to connect to.
        err = adc_oneshot_config_channel(adcHandle, toEspChannel(params().channel, error), &config);
        if (ESP_OK == err && ErrorType::Success == error) {
            const adc_cali_curve_fitting_config_t calibrationConfig = {
                .unit_id = toEspAdcUnitNumber(params().peripheralNumber, error),
                .chan = toEspChannel(params().channel, error),
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT
            };
            if (ErrorType::Success == error) {
                err = adc_cali_create_scheme_curve_fitting(&calibrationConfig, &calibrationHandle);
            }
        }
    }

    return fromPlatformError(err);
}

ErrorType Adc::deinit() {
    esp_err_t err = adc_oneshot_del_unit(adcHandle);
    if (ESP_OK == err) {
        err = adc_cali_delete_scheme_curve_fitting(calibrationHandle);
    }

    return fromPlatformError(err);
}

ErrorType Adc::convert(Count &rawValue) {
    assert(nullptr != adcHandle);

    int adcRawValue;
    ErrorType error = ErrorType::Failure;

    esp_err_t err = adc_oneshot_read(adcHandle, toEspChannel(params().channel, error), &adcRawValue);

    //Can the raw value be negative? Why is it an int?
    assert(adcRawValue >= 0);

    rawValue = adcRawValue;

    return fromPlatformError(err);
}

ErrorType Adc::rawToVolts(const Count rawValue, Volts &volts) {
    assert(nullptr != calibrationHandle);

    int milliVolts = 0;

    esp_err_t err = adc_cali_raw_to_voltage(calibrationHandle, rawValue, &milliVolts);
    volts = milliVolts / 1000.0f;
    return fromPlatformError(err);
}