//AbstractionLayer
#include "PowerResetClockManagementModule.hpp"
#include <Math.hpp>
//C++
#include <cassert>

ErrorType PowerResetClockManagement::init() {
    return ErrorType::NotImplemented;
}

ErrorType PowerResetClockManagement::setClockFrequency(const Hertz frequency, const Hertz externalCrystalFrequency) {
    ErrorType error = ErrorType::Failure;

    if (isValidFrequency(frequency)) {
        uint32_t clockConfig = SYSCTL_USE_PLL | SYSCTL_OSC_MAIN;

        const uint32_t sysCtlClockFrequency = toTm4c123SysCtlClockFrequency(frequency, true, true);
        const uint32_t sysCtlExternalClockFrequency = toTm4c123SysCtlExternalClockFrequency(externalCrystalFrequency, true);

        const bool frequenciesAreValid = (0 != sysCtlClockFrequency && 0 != sysCtlExternalClockFrequency);
        if (frequenciesAreValid) {
            clockConfig |= sysCtlClockFrequency;
            clockConfig |= sysCtlExternalClockFrequency;

            SysCtlClockSet(clockConfig);
            error = ErrorType::Success;
        }
    }
    else {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

uint32_t PowerResetClockManagement::toTm4c123SysCtlClockFrequency(const Hertz frequency, const bool usePll, const bool useMainOscillator) {
    if (isValidFrequency(frequency)) {
        if (usePll && useMainOscillator) {
            constexpr int32_t maxPllOutput = 200E6;
            const int32_t clockDivider = std::round(static_cast<float>(maxPllOutput) / frequency);

            const int32_t frequencyWithWholeNumberDivider = maxPllOutput / clockDivider;
            const int32_t frequencyWithHalfAddedToDivider = maxPllOutput / static_cast<float>(clockDivider + 0.5);
            const int32_t frequencyWithHalfSubtractedFromDivider = maxPllOutput / static_cast<float>(clockDivider - 0.5);

            const bool frequencyWithHalfAddedToDividerIsMostAccurate = (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfAddedToDivider)) < (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfSubtractedFromDivider)) &&
                                                                       (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfAddedToDivider)) < (std::abs(static_cast<int32_t>(frequency) - frequencyWithWholeNumberDivider));
            const bool frequencyWithHalfSubtractedFromDividerIsMostAccurate = (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfSubtractedFromDivider)) < (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfAddedToDivider)) &&
                                                                              (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfSubtractedFromDivider)) < (std::abs(static_cast<int32_t>(frequency) - frequencyWithWholeNumberDivider));
            const bool frequencyWithWholeNumberDividerIsMostAccurate = (std::abs(static_cast<int32_t>(frequency) - frequencyWithWholeNumberDivider)) < (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfAddedToDivider)) &&
                                                                       (std::abs(static_cast<int32_t>(frequency) - frequencyWithWholeNumberDivider)) < (std::abs(static_cast<int32_t>(frequency) - frequencyWithHalfSubtractedFromDivider));
            
            const bool onlyOneDividerIsMostAccurate = frequencyWithHalfAddedToDividerIsMostAccurate ^ frequencyWithHalfSubtractedFromDividerIsMostAccurate ^ frequencyWithWholeNumberDividerIsMostAccurate;
            assert(onlyOneDividerIsMostAccurate);

            if (frequencyWithHalfAddedToDividerIsMostAccurate) {
                return toTm4c123SysCtlDividerPlusHalf(clockDivider);
            }
            else if (frequencyWithHalfSubtractedFromDividerIsMostAccurate) {
                return toTm4c123SysCtlDividerMinusHalf(clockDivider);
            }
            else if (frequencyWithWholeNumberDividerIsMostAccurate) {
                return toTm4c123SysCtlDivider(clockDivider);
            }
        }
    }

    return 0;
}

uint32_t PowerResetClockManagement::toTm4c123SysCtlExternalClockFrequency(const Hertz externalCrystalFrequency, const bool usePll) {
    constexpr float tolerance = 0.01f;
    if (!usePll) {
        if (withinError(static_cast<float>(externalCrystalFrequency), 4E6f, tolerance)) {
            return SYSCTL_XTAL_4MHZ;
        }
        else if (withinError(static_cast<float>(externalCrystalFrequency), 4.090E6f, tolerance)) {
            return SYSCTL_XTAL_4_09MHZ;
        }
        else if (withinError(static_cast<float>(externalCrystalFrequency), 4.910E6f, tolerance)) {
            return SYSCTL_XTAL_4_91MHZ;
        }
    }
    if (withinError(static_cast<float>(externalCrystalFrequency), 5.000E6f, tolerance)) {
        return SYSCTL_XTAL_5MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 5.120E6f, tolerance)) {
        return SYSCTL_XTAL_5_12MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 6.000E6f, tolerance)) {
        return SYSCTL_XTAL_6MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 6.140E6f, tolerance)) {
        return SYSCTL_XTAL_6_14MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 7.370E6f, tolerance)) {
        return SYSCTL_XTAL_7_37MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 8.000E6f, tolerance)) {
        return SYSCTL_XTAL_8MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 8.190E6f, tolerance)) {
        return SYSCTL_XTAL_8_19MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 10.000E6f, tolerance)) {
        return SYSCTL_XTAL_10MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 12.000E6f, tolerance)) {
        return SYSCTL_XTAL_12MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 12.200E6f, tolerance)) {
        return SYSCTL_XTAL_12_2MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 13.500E6f, tolerance)) {
        return SYSCTL_XTAL_13_5MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 14.300E6f, tolerance)) {
        return SYSCTL_XTAL_14_3MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 16.000E6f, tolerance)) {
        return SYSCTL_XTAL_16MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 24.000E6f, tolerance)) {
        return SYSCTL_XTAL_24MHZ;
    }
    else if (withinError(static_cast<float>(externalCrystalFrequency), 25.000E6f, tolerance)) {
        return SYSCTL_XTAL_25MHZ;
    }
    else {
        return 0;
    }

    return 0;
}

constexpr uint32_t PowerResetClockManagement::toTm4c123SysCtlDivider(const uint32_t divider) {
    if (divider <= 2) {
        return SYSCTL_SYSDIV_2_5;
    }

    constexpr uint32_t scalar = (SYSCTL_SYSDIV_3 - SYSCTL_SYSDIV_2);
    //Note that SYSDIV 1 and 2 are reserved in the data sheet and not useable.
    //We subtract by 2 from the divider because we are starting at 2. To get a sysdiv of 3 we should scale by 1.
    uint32_t sysdiv = (SYSCTL_SYSDIV_2 + ((divider - 2) * scalar)) & ~(0xF0 << 24);

    if (divider > 15) {
        if (divider > 31) {
            sysdiv |= 0x90 << 24;
        }
        else {
            sysdiv |= 0x80 << 24;
        }
    }

    return sysdiv;
}

constexpr uint32_t PowerResetClockManagement::toTm4c123SysCtlDividerPlusHalf(const uint32_t divider) {
    if (divider <= 2) {
        return SYSCTL_SYSDIV_2_5;
    }

    const uint32_t wholeNumberDivider = toTm4c123SysCtlDivider(divider) & ~(0xF0 << 24);

    constexpr uint32_t scalar = SYSCTL_SYSDIV_2_5 - SYSCTL_SYSDIV_2;

    if (divider > 15) {
        if (divider > 31) {
            return (wholeNumberDivider + scalar) | (0xD0 << 24);
        }
        else {
            return (wholeNumberDivider + scalar) | (0xC0 << 24);
        }
    }

    return wholeNumberDivider + scalar;
}

constexpr uint32_t PowerResetClockManagement::toTm4c123SysCtlDividerMinusHalf(const uint32_t divider) {
    if (divider <= 2) {
        return SYSCTL_SYSDIV_2_5;
    }

    constexpr uint32_t scalar = (SYSCTL_SYSDIV_3_5 - SYSCTL_SYSDIV_2_5);

    return toTm4c123SysCtlDividerPlusHalf(divider) - scalar;
}