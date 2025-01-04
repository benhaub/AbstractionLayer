//AbstractionLayer
#include "Error.hpp"
#include "Log.hpp"
//TI driverlib
#include "ti/drivers/Power.h"
//TI driverlib
#include "ti/drivers/PWM.h"
#include "ti/drivers/net/wifi/errors.h"
#include "errno.h"

ErrorType fromPlatformError(int32_t err) {

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
    else if (SL_ERROR_ROLE_STA_ERR || SL_ERROR_ROLE_AP_ERR || SL_ERROR_ROLE_P2P_ERR || SL_ERROR_CALIB_FAIL || SL_ERROR_FS_CORRUPTED_ERR || SL_ERROR_FS_ALERT_ERR ||
             SL_ERROR_RESTORE_IMAGE_COMPLETE || SL_ERROR_ROLE_TAG_ERR || SL_ERROR_FIPS_ERR || SL_ERROR_GENERAL_ERR) {
                return ErrorType::Failure;
    }
    else if (SL_RET_CODE_DEV_ALREADY_STARTED) {
        return ErrorType::Success;
    }
    else if (err == ENOSPC) {
        return ErrorType::NoMemory;
    }
    else if (err == EINVAL) {
        return ErrorType::InvalidParameter;
    }
    else {
        PLT_LOGW("ErrnoError", "Got unhandled error code %d", err);
        return ErrorType::InvalidParameter;
    }
}