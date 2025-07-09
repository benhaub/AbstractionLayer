//AbstractionLayer
#include "RtcManager.hpp"
#include "Math.hpp"
#include "Log.hpp"

ErrorType RtcManager::submitInputTime(const UnixTime &inputTime) {
    ErrorType error;
    DateTime internalTime;
    error = internalRtcTime(internalTime);
    if (ErrorType::Success == error) {
        Seconds differenceBetweenInternalRtcAndInputTime;
        const UnixTime internalUnixTime = ToUnixTime(internalTime);

        if (inputTime <= internalUnixTime) {
            differenceBetweenInternalRtcAndInputTime = internalUnixTime - inputTime;
        }
        else {
            differenceBetweenInternalRtcAndInputTime = inputTime - internalUnixTime;
        }

        if (differenceBetweenInternalRtcAndInputTime > _criteria.maxDifferenceBetweenInternalRtcAndInputTime) {
            if (nullptr != _internalRtc.get()) {
                PLT_LOGI(TAG, "Writing time to internal RTC <time:%s>", ToDateTime(inputTime).toString().data());
                error = _internalRtc->writeDate(ToDateTime(inputTime));
            }
        }

        const UnixTime externalUnixTime = ToUnixTime(_lastExternalRtcTimeQueried);
        Seconds differenceBetweenInternalRtcAndExternalRtc;
        if (internalUnixTime > externalUnixTime) {
            differenceBetweenInternalRtcAndExternalRtc = internalUnixTime - externalUnixTime;
        }
        else {
            differenceBetweenInternalRtcAndExternalRtc = externalUnixTime - internalUnixTime;
        }

        if (differenceBetweenInternalRtcAndExternalRtc > _criteria.maxDifferenceBetweenInternalAndExternalRtc) {
            if (nullptr != _externalRtc.get()) {
                PLT_LOGI(TAG, "Writing time to external RTC <time:%s>", ToDateTime(inputTime).toString().data());
                error = _externalRtc->writeDate(ToDateTime(inputTime));
                if (ErrorType::Success == error) {
                    error = _externalRtc->readDate(_lastExternalRtcTimeQueried);
                }
            }
        }
    }

    return error;
}

ErrorType RtcManager::internalRtcTime(DateTime &dateTime) const {
    if (nullptr != _internalRtc.get()) {
        return _internalRtc->readDate(dateTime);
    }
    else {
        return ErrorType::NoData;
    }
}

ErrorType RtcManager::externalRtcTime(const bool forceQuery, DateTime &dateTime) {
    if (forceQuery) {
        ErrorType error = _externalRtc->readDate(dateTime);
        if (ErrorType::Success == error) {
            _lastExternalRtcTimeQueried = dateTime;
        }

        return error;
    }
    else {
        dateTime = _lastExternalRtcTimeQueried;
        return ErrorType::Success;
    }
}