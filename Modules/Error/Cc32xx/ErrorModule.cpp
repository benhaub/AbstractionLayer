//AbstractionLayer
#include "Error.hpp"
#include "Log.hpp"
//TI driverlib
#include "ti/drivers/Power.h"
//TI driverlib
#include "ti/drivers/PWM.h"

ErrorType toPlatformError(int32_t err) {

    //A switch statement would be nice but there are duplicate error values.
    if (err == Power_SOK || err == PWM_STATUS_SUCCESS) {
        return ErrorType::Success;
    }
    else if (err == Power_EFAIL || err == PWM_STATUS_ERROR) {
        return ErrorType::Failure;
    }
    else if (err == Power_EINVALIDINPUT || err == Power_EINVALIDPOINTER || err == PWM_STATUS_UNDEFINEDCMD || err == PWM_STATUS_INVALID_DUTY) {
        return ErrorType::InvalidParameter;
    }
    else if (err == Power_ECHANGE_NOT_ALLOWED || err == Power_EBUSY) {
        return ErrorType::PrerequisitesNotMet;
    }
    else {
        PLT_LOGW("ErrnoError", "Got unhandled error code %d", err);
        return ErrorType::InvalidParameter;
    }
}