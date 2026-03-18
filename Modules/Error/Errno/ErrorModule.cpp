//AbstractionLayer
#include "Error.hpp"
#include "Log.hpp"
//C++
#include <cerrno>

ErrorType fromPlatformError(int32_t err) {

    switch (err) {
        case -1:
            return ErrorType::Failure;
        case 0:
            return ErrorType::Success;
        case ENOMEM:
            return ErrorType::NoMemory;
        case ETIME:
        case ETIMEDOUT:
            return ErrorType::Timeout;
        case ENOSYS:
            return ErrorType::NotSupported;
        case EACCES:
        case EPERM:
        case EDEADLK:
        case EPIPE:
        case ECONNRESET:
            return ErrorType::PrerequisitesNotMet;
        case EINVAL:
        case ENOTSOCK:
        case EBADF:
        case ENAMETOOLONG:
            return ErrorType::InvalidParameter;
        case ENOENT:
            return ErrorType::FileNotFound;
        case EEXIST:
            return ErrorType::FileExists;
        case ENODATA:
        case ESRCH:
            return ErrorType::NoData;
        case EAGAIN:
        case EOVERFLOW:
        case EADDRINUSE:
            return ErrorType::LimitReached;
        default:
            PLT_LOGW("ErrnoError", "Got unhandled error code %d", err);
            return ErrorType::Failure;
    }
}