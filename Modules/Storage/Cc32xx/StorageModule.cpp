//AbstractionLayer
#include "StorageModule.hpp"
#include "SpiModule.hpp"
#include "OperatingSystemModule.hpp"
//TI driverlib
#include "ti/drivers/net/wifi/fs.h"

ErrorType Storage::init() {
    Spi spi;
    Id thread;
    ErrorType error = spi.init();
    if (ErrorType::Success != error) {
        return error;
    }

    constexpr Bytes kilobyte = 1024;
    //SIMPLELINK_THREAD_NAME is defined at compile time in cc32xx.cmake. Both Wifi and storage must create this thread in order to access the
    //API for Wifi and storage and may do so independently of each other. The Operating System must be check for this presence of this thread
    //before attempting to start it. Checking the return value of sl_start() is not sufficient since the wifi radio can be turned on or off at
    //any time.
    constexpr std::array<char, OperatingSystemTypes::MaxThreadNameLength> simplelinkThreadName = {SIMPLELINK_THREAD_NAME};
    if (ErrorType::NoData == OperatingSystem::Instance().threadId(simplelinkThreadName, thread)) {
        
        error = OperatingSystem::Instance().createThread(OperatingSystemTypes::Priority::High,
                                                simplelinkThreadName,
                                                nullptr,
                                                2*kilobyte,
                                                sl_Task,
                                                thread);
        
        assert(ErrorType::Success == error);
    }

    sl_Start(NULL, NULL, NULL);
    _status.isInitialized = true;

    return error;
} 

ErrorType Storage::deinit() {
    bool deinitializationDone = false;
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

    if (!deinitializationDone) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType Storage::mainLoop() {
    return runNextEvent();
}
