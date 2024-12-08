//AbstractionLayer
#include "Error.hpp"
#include "Log.hpp"
//TI driverlib
#include "ti/drivers/Power.h"

ErrorType toPlatformError(int32_t err) {

    switch (err) {
        case Power_SOK:
            return ErrorType::Success;
        case Power_EFAIL:
            return ErrorType::Failure;
        case Power_EINVALIDINPUT:
        case Power_EINVALIDPOINTER:
            return ErrorType::InvalidParameter;
        case Power_ECHANGE_NOT_ALLOWED:
        case Power_EBUSY:
            return ErrorType::PrerequisitesNotMet;
        default:
            CBT_LOGW("ErrnoError", "Got unhandled error code %d", err);
            return ErrorType::InvalidParameter;
    }
}