//AbstractionLayer
#include "Error.hpp"

ErrorType fromPlatformError(int32_t err) {

    switch (err) {
        default:
            return ErrorType::NotImplemented;
    }
}