//AbstractionLayer
#include "Error.hpp"
#include "Log.hpp"
//TI driverlib
#include "ti/drivers/Power.h"
//TI driverlib
#include "ti/drivers/PWM.h"
#include "ti/drivers/net/wifi/errors.h"
#include "errno.h"

//TI does not document all their error values very well. A good trick to find more information is to grep it in the SDK docs
//and see where it appears in there and then go look at those pages to see if there is any more information.
ErrorType fromPlatformError(int32_t err) {

    //A switch statement would be nice but there are duplicate error values.
    if (err == Power_SOK || err == PWM_STATUS_SUCCESS || err == SL_FS_OK) {
        return ErrorType::Success;
    }
    else if (err == SL_ERROR_NET_APP_DNS_ERROR || err == SL_ERROR_NET_APP_DNS_QUERY_NO_RESPONSE || err == Power_EFAIL || err == PWM_STATUS_ERROR) {
        return ErrorType::Failure;
    }
    else if (err == Power_EINVALIDINPUT || err == Power_EINVALIDPOINTER || err == PWM_STATUS_UNDEFINEDCMD || err == PWM_STATUS_INVALID_DUTY ||
             err == SL_ERROR_FS_INVALID_HANDLE) {
        return ErrorType::InvalidParameter;
    }
    else if (err == Power_ECHANGE_NOT_ALLOWED || err == Power_EBUSY || err == SL_RET_CODE_PROVISIONING_IN_PROGRESS || err == SL_RET_CODE_DEV_NOT_STARTED ||
             err == SL_ERROR_NET_APP_DNS_NO_SERVER) {
        return ErrorType::PrerequisitesNotMet;
    }
    else if (err == SL_ERROR_ROLE_STA_ERR || err == SL_ERROR_ROLE_AP_ERR || err == SL_ERROR_ROLE_P2P_ERR || err == SL_ERROR_CALIB_FAIL || err == SL_ERROR_FS_CORRUPTED_ERR || err == SL_ERROR_FS_ALERT_ERR ||
             err == SL_ERROR_RESTORE_IMAGE_COMPLETE || err == SL_ERROR_ROLE_TAG_ERR || err == SL_ERROR_FIPS_ERR || err == SL_ERROR_GENERAL_ERR) {
                return ErrorType::Failure;
    }
    else if (err == SL_RET_CODE_DEV_ALREADY_STARTED || err == SL_ERROR_FS_FILE_IS_ALREADY_OPENED) {
        return ErrorType::Success;
    }
    else if (err == ENOSPC || err == SL_ERROR_BSD_ENOMEM) {
        return ErrorType::NoMemory;
    }
    else if (err == EINVAL || err == SL_ERROR_WLAN_PROVISIONING_ABORT_INVALID_PARAM) {
        return ErrorType::InvalidParameter;
    }
    else if (err == SL_ERROR_FS_FILE_ALREADY_EXISTS) {
        return ErrorType::FileExists;
    }
    else if (err == SL_ERROR_FS_FILE_NOT_EXISTS) {
        return ErrorType::FileNotFound;
    }
    else if (SL_ERROR_BSD_EAGAIN) {
        return ErrorType::Timeout;
    }
    else {
        PLT_LOGW("ErrnoError", "Got unhandled error code %d", err);
        return ErrorType::InvalidParameter;
    }
}