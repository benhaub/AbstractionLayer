//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "Math.hpp"
#include "ProcessorModule.hpp"
//C++
#include <ctime>
//Posix
#include <pthread.h>
//TI driverlib
#include "ti/drivers/net/wifi/device.h"

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(TimerHandle_t timer);

#ifdef __cplusplus
}
#endif

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    const Milliseconds minimumDelayToBlock = delay * (1000 / configTICK_RATE_HZ);
    vTaskDelay(minimumDelayToBlock);
    return ErrorType::Success;
}

ErrorType OperatingSystem::delay(const Microseconds delay) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::startScheduler() {
    vTaskStartScheduler();

    //Never returns
    return ErrorType::Failure;
}

ErrorType OperatingSystem::createThread(const OperatingSystemTypes::Priority priority, const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    ErrorType error = ErrorType::LimitReached;
    static Id nextThreadId = OperatingSystemTypes::NullId + 1;

    //Unlike other modules which use InitThreadArgs and initThread as a means to fully register the thread with the operating system before
    //the start function is called, this module uses the lambda as a wrapper for the function pointer since there is a conflict in the types
    //that the abstraction layer uses vs. what FreeRTOS expects.
    struct InitThreadArgs {
        void *arguments;
        void *(*startFunction)(void *);
    };
    auto initThread = [](void *arguments) -> void {
        InitThreadArgs *initThreadArgs = static_cast<InitThreadArgs *>(arguments);
        void *threadArguments = initThreadArgs->arguments;
        void *(*startFunction)(void *) = initThreadArgs->startFunction;
        delete initThreadArgs;
        initThreadArgs = nullptr;
        (startFunction)(threadArguments);
        return;
    };

    InitThreadArgs *initThreadArgs = new InitThreadArgs {
        .arguments = arguments,
        .startFunction = startFunction,
    };

    TaskHandle_t thread;
    const bool threadWasCreated = (pdPASS == xTaskCreate(initThread, name.data(), stackSize/4, initThreadArgs, toCc32xxPriority(priority), &thread));

    if (threadWasCreated) {
#if INCLUDE_uxTaskGetStackHighWaterMark
        OperatingSystemTypes::MemoryRegionInfo stackRegion = {name};
        _status.memoryRegion.push_back(stackRegion);
#endif
        threads.at(toThreadIndex(nextThreadId)).name = name;
        threads.at(toThreadIndex(nextThreadId)).threadId = nextThreadId;
        threads.at(toThreadIndex(nextThreadId)).maxStackSize = stackSize;
        threads.at(toThreadIndex(nextThreadId)).cc32xxThreadId = thread;
        threads.at(toThreadIndex(nextThreadId)).status = OperatingSystemTypes::ThreadStatus::Active;
        number = threads.at(toThreadIndex(nextThreadId)).threadId;
        _status.threadCount++;
        nextThreadId++;
        error = ErrorType::Success;
    }
    else {
        deleteThread(name);
        error = ErrorType::Failure;
    }

    return error;
}

ErrorType OperatingSystem::deleteThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    ErrorType error = ErrorType::NoData;

    auto it = std::find_if(threads.begin(), threads.end(), [name](const auto &thread) { return 0 == strncmp(thread.name.data(), name.data(), OperatingSystemTypes::MaxThreadNameLength); });
    if (threads.end() == it) {
        return ErrorType::NoData;
    }
    else {
        it->status = OperatingSystemTypes::ThreadStatus::Terminated;
        // Remove thread stack from memory regions
        _status.memoryRegion.erase(
            std::remove_if(_status.memoryRegion.begin(), _status.memoryRegion.end(),
                [&name](const OperatingSystemTypes::MemoryRegionInfo &region) {
                    return 0 == strncmp(region.name.data(), name.data(), OperatingSystemTypes::MaxMemoryRegionNameLength);
                }),
            _status.memoryRegion.end());
        
        _status.threadCount--;
        error = ErrorType::Success;
    }

    return error;
}

ErrorType OperatingSystem::joinThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    while (ErrorType::NoData != isDeleted(name)) {
        delay(Milliseconds(1));
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::threadId(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, Id &thread) {
    auto it = std::find_if(threads.begin(), threads.end(), [name](const auto &thread) { return 0 == strncmp(thread.name.data(), name.data(), OperatingSystemTypes::MaxThreadNameLength); });
    if (threads.end() == it) {
        return ErrorType::NoData;
    }

    thread = it->threadId;
    return ErrorType::Success;
}

ErrorType OperatingSystem::currentThreadId(Id &thread) const {
    const TaskHandle_t threadId = xTaskGetCurrentTaskHandle();

    auto it = std::find_if(threads.begin(), threads.end(), [threadId](const auto &thread) { return thread.cc32xxThreadId == threadId; });
    if (threads.end() == it) {
        thread = OperatingSystemTypes::NullId;
        return ErrorType::NoData;
    }

    thread = it->threadId;

    return ErrorType::Success;
}

ErrorType OperatingSystem::isDeleted(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    Id thread;

    if (ErrorType::NoData != threadId(name, thread)) {
        auto it = std::find_if(threads.begin(), threads.end(), [&](const auto &thread) {
            return thread.status == OperatingSystemTypes::ThreadStatus::Terminated &&
            thread.name == name; 
        });

        if (threads.end() == it) {
            return ErrorType::Negative;
        }
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::createSemaphore(const Count max, const Count initial, const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    SemaphoreHandle_t freertosSemaphore;

    freertosSemaphore = xSemaphoreCreateCounting(max, initial);

    if (nullptr == freertosSemaphore) {
        return ErrorType::NoMemory;
    }

    semaphores[name] = freertosSemaphore;

    return ErrorType::Success;
}

ErrorType OperatingSystem::deleteSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    vSemaphoreDelete(semaphores[name]);
    semaphores.erase(name);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name, const Milliseconds timeout) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreTake(semaphores[name], timeout)) {
        return ErrorType::Success;
    }

    return ErrorType::Timeout;
}

ErrorType OperatingSystem::incrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreGive(semaphores[name])) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::decrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreTake(semaphores[name], 0)) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::createTimer(Id &timer, const Milliseconds period, const bool autoReload, std::function<void(void)> callback) {
    TimerHandle_t timerHandle = nullptr;
    Timer newTimer = {
        .callback = callback,
        .id = nextTimerId++,
        .autoReload = autoReload
    };

    if (nullptr != (timerHandle = xTimerCreate(nullptr, pdMS_TO_TICKS(period), autoReload, nullptr, TimerCallback))) {
        timers[timerHandle] = newTimer;
        timer = newTimer.id;
        return ErrorType::Success;
    }
    else {
        deleteTimer(timer);
        return ErrorType::Failure;
    }
}

ErrorType OperatingSystem::deleteTimer(const Id timer) {
   auto itr = timers.begin();

    while (itr != timers.end()) {
        if (timer == itr->second.id) {
            xTimerDelete(itr->first, 0);
            timers.erase(itr);
            return ErrorType::Success;
        }
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::startTimer(const Id timer, const Milliseconds timeout) {
    auto itr = timers.begin();
    while(itr != timers.end()) {
        if (itr->second.id == timer) {
            if (pdTRUE == xTimerStart(itr->first, pdMS_TO_TICKS(timeout))) {
                return ErrorType::Success;
            }
        }
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::stopTimer(const Id timer, const Milliseconds timeout) {
    auto itr = timers.begin();
    while(itr != timers.end()) {
        if (itr->second.id == timer) {
            if (pdTRUE == xTimerStop(itr->first, pdMS_TO_TICKS(timeout))) {
                return ErrorType::Success;
            }
        }
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::createQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const Bytes size, const Count length) {
    QueueHandle_t handle = nullptr;

    handle = xQueueCreate(length, size);

    if (nullptr == handle) {
        return ErrorType::NoMemory;
    }

    queues[name] = handle;

    return ErrorType::Success;
}

ErrorType OperatingSystem::sendToQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) {
    if (!queues.contains(name)) {
        return ErrorType::NoData;
    }

    BaseType_t sent = pdTRUE;

    if (toFront) {
        if (fromIsr) {
            sent = xQueueSendToFrontFromISR(queues[name], data, NULL);
        }
        else {
            Ticks ticks;
            millisecondsToTicks(timeout, ticks);
            sent = xQueueSendToFront(queues[name], data, ticks);
        }
    }
    else {
        if (fromIsr) {
            sent = xQueueSendFromISR(queues[name], data, NULL);
        }
        else {
            Ticks ticks;
            millisecondsToTicks(timeout, ticks);
            sent = xQueueSend(queues[name], data, ticks);
        }
    }

    if (pdTRUE == sent) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::Failure;
    }
}

ErrorType OperatingSystem::receiveFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
    if (!queues.contains(name)) {
        return ErrorType::NoData;
    }

    BaseType_t sent = pdTRUE;

    if (fromIsr) {
        sent = xQueueReceiveFromISR(queues[name], buffer, NULL);
    }
    else {
        Ticks ticks;
        millisecondsToTicks(timeout, ticks);
        sent = xQueueReceive(queues[name], buffer, ticks);
    }

    if (pdTRUE == sent) {
        return ErrorType::Success;
    }
    else if (uxQueueMessagesWaiting(queues[name]) > 0) {
        return ErrorType::Failure;
    }
    else {
        return ErrorType::Timeout;
    }
}

ErrorType OperatingSystem::peekFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
    if (!queues.contains(name)) {
        return ErrorType::NoData;
    }

    BaseType_t sent = pdTRUE;

    if (fromIsr) {
        sent = xQueuePeekFromISR(queues[name], buffer);
    }
    else {
        Ticks ticks;
        millisecondsToTicks(timeout, ticks);
        sent = xQueuePeek(queues[name], buffer, ticks);
    }

    if (pdTRUE == sent) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::Failure;
    }
}

ErrorType OperatingSystem::getSystemTime(UnixTime &currentSystemUnixTime) {
    currentSystemUnixTime = static_cast<UnixTime>(time(nullptr));
    return ErrorType::Success;
}

ErrorType OperatingSystem::getSystemTick(Ticks &currentSystemTick) {
    currentSystemTick = static_cast<Ticks>(xTaskGetTickCount());
    return ErrorType::Success;
}

ErrorType OperatingSystem::ticksToMilliseconds(const Ticks ticks, Milliseconds &timeInMilliseconds) {
    timeInMilliseconds = static_cast<Ticks>((1000 * ticks) / configTICK_RATE_HZ);
    return ErrorType::Success;
}

ErrorType OperatingSystem::millisecondsToTicks(const Milliseconds milli, Ticks &ticks) {
    ticks = static_cast<Milliseconds>(pdMS_TO_TICKS(milli));
    return ErrorType::Success;
}

ErrorType OperatingSystem::getSoftwareVersion(std::string &softwareVersion) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::getResetReason(OperatingSystemTypes::ResetReason &resetReason) {
    //There isn't really such thing as a reset reason for most posix systems, so we'll just call it power-on.
    resetReason = OperatingSystemTypes::ResetReason::PowerOn;
    return ErrorType::Success;
}

ErrorType OperatingSystem::reset() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const int16_t timeZoneDifferenceUtc) {
    struct timespec timeSpec;
    timeSpec.tv_sec = utc + timeZoneDifferenceUtc;
    timeSpec.tv_nsec = 0;

    return fromPlatformError(clock_settime(CLOCK_REALTIME, &timeSpec));
}

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
    _status.idle = idlePercent;
    return ErrorType::Success;
}

ErrorType OperatingSystem::memoryRegionUsage(OperatingSystemTypes::MemoryRegionInfo &region) {
    ErrorType error = ErrorType::NoData;
    constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> heap = {"Heap"};

    if (0 == strncmp(region.name.data(), heap.data(), OperatingSystemTypes::MaxMemoryRegionNameLength)) {
        Bytes totalHeap = configTOTAL_HEAP_SIZE;
        Bytes freeHeap = xPortGetFreeHeapSize();
        region.free = (totalHeap > 0) ? ((float)freeHeap / totalHeap) * 100.0f : 0.0f;
    }
#if INCLUDE_uxTaskGetStackHighWaterMark
    else {
        for (const auto &thread : threads) {

            if (0 == strncmp(region.name.data(), thread.name.data(), OperatingSystemTypes::MaxMemoryRegionNameLength)) {

                if (thread.cc32xxThreadId != nullptr) {
                    UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(thread.cc32xxThreadId) * 4;
                    region.free = (thread.maxStackSize > 0) ? (Percent(stackHighWaterMark) / thread.maxStackSize) * 100.0f : 0.0f;
                }

                error = ErrorType::Success;
            }
        }
    }
#endif

    return error;
}

ErrorType OperatingSystem::uptime(Seconds &uptime) {
    static Seconds sinceLastRollover = 0;

    Ticks systemNow;
    getSystemTick(systemNow);
    Milliseconds uptimeNow;
    ticksToMilliseconds(systemNow, uptimeNow);

    const bool tickCountHasRolledOver = (uptimeNow / 1000) < sinceLastRollover;
    if (tickCountHasRolledOver) {
        sinceLastRollover = 0;
        _status.upTime += (uptimeNow / 1000);
    }
    else {
        _status.upTime += (uptimeNow / 1000) - sinceLastRollover;
        sinceLastRollover = uptimeNow / 1000;
    }


    return ErrorType::Success;
}

ErrorType OperatingSystem::disableAllInterrupts() {
    if (ErrorType::Success == Processor::Instance().isInterruptContext()) {
        assert(savedInterruptContexts.size() < savedInterruptContexts.capacity() && "Can not allocate memory from interrupt context");
        savedInterruptContexts.push_back(taskENTER_CRITICAL_FROM_ISR());
    }
    else {
        taskENTER_CRITICAL();
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::enableAllInterrupts() {
    if (ErrorType::Success == Processor::Instance().isInterruptContext()) {
        taskEXIT_CRITICAL_FROM_ISR(savedInterruptContexts.back());
        savedInterruptContexts.pop_back();
    }
    else {
        taskEXIT_CRITICAL();
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::block() {
    Id task;
    ErrorType error = currentThreadId(task);

    if (OperatingSystemTypes::NullId != task) {

        for (auto &threadStruct : threads) {

            if (threadStruct.threadId == task) {
                constexpr BaseType_t clearCountOnReturn = pdTRUE;
                constexpr BaseType_t waitForever = portMAX_DELAY;

                threadStruct.status = OperatingSystemTypes::ThreadStatus::Blocked;

                const BaseType_t numberOfTimsPerviouslyUnblocked = ulTaskNotifyTake(clearCountOnReturn, waitForever);
                const bool threadHasBeenPreviouslyUnblocked = numberOfTimsPerviouslyUnblocked >= 1;

                if (threadHasBeenPreviouslyUnblocked) {
                    error = ErrorType::LimitReached;
                }

                threadStruct.status = OperatingSystemTypes::ThreadStatus::Active;

                break;
            }
        }
    }

    return error;
}

ErrorType OperatingSystem::unblock(const Id task) {
    ErrorType error = ErrorType::NoData;

    if (OperatingSystemTypes::NullId != task) {

        for (auto &threadStruct : threads) {

            if (threadStruct.threadId == task) {

                if (Processor::Instance().isInterruptContext() == ErrorType::Success) {
                    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                    vTaskNotifyGiveFromISR(threadStruct.cc32xxThreadId, &xHigherPriorityTaskWoken);
                }
                else {
                    xTaskNotifyGive(threadStruct.cc32xxThreadId);
                }

                threadStruct.status = OperatingSystemTypes::ThreadStatus::Active;

                error = ErrorType::Success;

                break;
            }
        }
    }

    return error;
}

void OperatingSystem::callTimerCallback(TimerHandle_t timer) {
    if (timers.contains(timer)) {
        timers[timer].callback();

        const bool timerIsOneShot = !timers[timer].autoReload;
        if (timerIsOneShot) {
            deleteTimer(timers[timer].id);
        }
    }

    return;
}

//From both observation (of the simple link example programs) and testing done while converting from pthreads to FreeRTOS task creation,
//the sl_Task can not be created using xTaskCreate. It must be created using pthreads.
ErrorType OperatingSystem::startSimpleLinkTask() {
    pthread_attr_t attrs;
    struct sched_param priParam;
    int retc;
    ErrorType error = ErrorType::Success;
    static bool slTaskHasNotBeenCreated = true;

    if (slTaskHasNotBeenCreated) {
        /* Initialize the attributes structure with default values */
        pthread_attr_init(&attrs);

        /* Set priority, detach state, and stack size attributes */
        priParam.sched_priority = toCc32xxPriority(OperatingSystemTypes::Priority::High);
        retc = pthread_attr_setschedparam(&attrs, &priParam);
        retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        retc |= pthread_attr_setstacksize(&attrs, 2*1024);

        if (0 == retc) {
            pthread_t thread;
            const uint32_t ret = pthread_create(&thread, &attrs, sl_Task, NULL);

            if (0 == ret) {
                slTaskHasNotBeenCreated = true;
            }
            else {
                error = ErrorType::Failure;
            }
        }
    }

    return error;
}

ErrorType OperatingSystem::getSystemMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    return ErrorType::NotAvailable;
}

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(TimerHandle_t timer) {
    OperatingSystem::Instance().callTimerCallback(timer);
}

void vApplicationIdleHook() {
    static Count numberOfTimesIdleHookHasBeenCalled = 1;
    static Ticks timeOfLastIdleHookCall = static_cast<Ticks>(xTaskGetTickCount());
    static Ticks averageTimeBetweenIdleHookCalls = static_cast<Ticks>(xTaskGetTickCount());

    const Ticks timeNow = static_cast<Ticks>(xTaskGetTickCount());

    //Handle overflow
    const Ticks timeBetweenIdleHookCalls = differenceBetween(timeNow, timeOfLastIdleHookCall);

    //The larger the time betwen calls, the more time we spent not in idle.
    averageTimeBetweenIdleHookCalls = runningAverage(averageTimeBetweenIdleHookCalls, timeBetweenIdleHookCalls, numberOfTimesIdleHookHasBeenCalled);
    const Ticks timeNotSpentInIdle = averageTimeBetweenIdleHookCalls * numberOfTimesIdleHookHasBeenCalled;
    Percent idle = ((1.0f - (float)timeNotSpentInIdle / timeNow) * 100.0f);
    OperatingSystem::Instance().idlePercentage(idle);

    numberOfTimesIdleHookHasBeenCalled++;

    timeOfLastIdleHookCall = timeNow;
}

#ifdef __cplusplus
}
#endif