//AbstractionLayer
#include "StorageModule.hpp"
#include "SpiModule.hpp"
#include "OperatingSystemModule.hpp"
//TI driverlib
#include "ti/drivers/net/wifi/fs.h"

ErrorType Storage::init() {
    Spi spi;
    ErrorType error = spi.init();

    if (ErrorType::Success == error) {
        error = OperatingSystem::Instance().startSimpleLinkTask();

        if (ErrorType::Success == error) {
            const signed short result = sl_Start(NULL, NULL, NULL);

            if (result < 0) {
                error = fromPlatformError(result);
            }
            else {
                _status.isInitialized = true;
                error = ErrorType::Success;
            }
        }
    }

    return error;
} 

ErrorType Storage::deinit() {
    volatile bool deinitializationDone = false;
    ErrorType callbackError = ErrorType::Failure;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto deinitializedCallback = [&, thread]() -> ErrorType {
        signed short result = sl_Stop(0xFFFF);
        if (0 == result) {
            _status.isInitialized = false;
        }

        deinitializationDone = true;
        OperatingSystem::Instance().unblock(thread);
        return fromPlatformError(result);
    };

    EventQueue::Event event = EventQueue::Event(std::bind(deinitializedCallback));
    ErrorType error = addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!deinitializationDone && ErrorType::LimitReached == OperatingSystem::Instance().block());

    return callbackError;
}
