/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Error.hpp
* @details Error types.
* @addtogroup Abstractions
* Common interface for software components.
* @addtogroup Applications
* Cross-platform applications of the AbstractionLayer that utilize the modules.
* @addtogroup Utilities
* Software that is used internally by the AbstractionLayer.
*******************************************************************************/

#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include <cstdint>

/**
 * @enum ErrorType
 * @brief Error types
*/
enum class ErrorType : uint8_t {
    Success             = 0,    ///< Nothing to worry about. Move on!
    Failure             = 1,    ///< General failure. Try to be more specific if you can.
    NotImplemented      = 2,    ///< For software that returns because the implementation does not exist for the given platform.
    NoMemory            = 3,    ///< Not enough memory to complete the operation
    Timeout             = 4,    ///< Operation timed out
    NotSupported        = 5,    ///< The operation is implemented, but doesn't support the provided data types. You couldn't implement the code even if you wanted to.
    PrerequisitesNotMet = 6,    ///< One or more prerequisites are not met. Read the inline documentation for more information.
    InvalidParameter    = 7,    ///< One or more parameters are invalid. Read the inline documentation for more information.
    CrcMismatch         = 8,    ///< Cyclic Redundancy Check failed.
    NotAvailable        = 9,    ///< The operation makes sense to be implemented, but the underlying software doesn't support this operation.
    FileNotFound        = 10,   ///< File not found.
    FileExists          = 11,   ///< File already exists.
    EndOfFile           = 12,   ///< End of file reached.
    NoData              = 13,   ///< No data available.
    LimitReached        = 14,   ///< An imposed limit has been reached.
    Negative            = 15    ///< The status or query is not true with respect to the predicate
};

/**
 * @brief returns an ErrorType from a platform specific error code
 * @param error The error code to convert 
 * @returns The ErrorType corresponding to the error code 
 */
ErrorType fromPlatformError(int32_t error);

#endif // __ERROR_HPP__
