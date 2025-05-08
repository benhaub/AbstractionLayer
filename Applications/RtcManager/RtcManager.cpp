//AbstractionLayer
#include "RtcManager.hpp"
#include "Math.hpp"

ErrorType RtcManager::submitInputTime(const DateTime &inputTime) {
    ErrorType error;
    DateTime internalTime;
    error = internalRtcTime(internalTime);
    Seconds differenceBetweenInternalRtcAndInputTime;
    const UnixTime inputUnixTime = ToUnixTime(inputTime);
    const UnixTime internalUnixTime = ToUnixTime(internalTime);
    if (inputTime <= internalTime) {
        differenceBetweenInternalRtcAndInputTime = internalUnixTime - inputUnixTime;
    }
    else {
        differenceBetweenInternalRtcAndInputTime = inputUnixTime - internalUnixTime;
    }

    if (differenceBetweenInternalRtcAndInputTime > _criteria.maxDifferenceBetweenInternalRtcAndInputTime) {
        if (nullptr != _internalRtc.get()) {
            error = _internalRtc->writeDate(inputTime);
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
            error = _externalRtc->writeDate(inputTime);
            if (ErrorType::Success == error) {
                error = _externalRtc->readDate(_lastExternalRtcTimeQueried);
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