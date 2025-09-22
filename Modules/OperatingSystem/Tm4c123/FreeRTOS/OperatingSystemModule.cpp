//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "Math.hpp"
#include "ProcessorModule.hpp"
//C++
#include <ctime>

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(TimerHandle_t timer);

#ifdef __cplusplus
}
#endif

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    vTaskDelay(delay);
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
    static Id nextThreadId = 1;

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

    Thread newThread = {
        .name = name,
        .threadId = nextThreadId++
    };

    InitThreadArgs *initThreadArgs = new InitThreadArgs {
        .arguments = arguments,
        .startFunction = startFunction,
    };

    TaskHandle_t thread;
    const bool threadWasCreated = (pdPASS == xTaskCreate(initThread, name.data(), stackSize/4, initThreadArgs, toTm4c123Priority(priority), &thread));

    if (threadWasCreated && threads.size() < _MaxThreads) {
        newThread.tm4c123ThreadId = thread;
        threads[name] = newThread;
        _status.threadCount = threads.size();
        number = newThread.threadId;
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

    if (threads.contains(name)) {
        threads.erase(name);
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
    if (threads.contains(name)) {
        thread = threads[name].threadId;
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::currentThreadId(Id &thread) const {
    const TaskHandle_t threadId = xTaskGetCurrentTaskHandle();

    auto it = std::find_if(threads.begin(), threads.end(), [threadId](const auto &pair) { return pair.second.tm4c123ThreadId == threadId; });
    if (threads.end() == it) {
        return ErrorType::NoData;
    }

    thread = it->second.threadId;

    return ErrorType::Success;
}

ErrorType OperatingSystem::isDeleted(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    if (threads.contains(name)) {
        return ErrorType::Success;
    }

    return ErrorType::NoData;
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
    auto it = std::find_if(timers.begin(), timers.end(),
        [timer](const auto& pair) { return pair.second.id == timer; });
    
    if (it == timers.end()) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xTimerStart(it->first, pdMS_TO_TICKS(timeout))) {
        return ErrorType::Success;
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
    return ErrorType::NotImplemented;
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
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
    _status.idle = idlePercent;
    return ErrorType::Success;
}

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) {
    size = configTOTAL_HEAP_SIZE;
    return ErrorType::Success;
}

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) {
    size = xPortGetFreeHeapSize();
    return ErrorType::Success;
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

    for (auto &[name, threadStruct] : threads) {

        if (threadStruct.threadId == task) {
            vTaskSuspend(threadStruct.tm4c123ThreadId);
            error = ErrorType::Success;
            break;
        }
    }

    return error;
}

ErrorType OperatingSystem::unblock(const Id task) {
    ErrorType error = ErrorType::NoData;

    for (auto &[name, threadStruct] : threads) {

        if (threadStruct.threadId == task) {
            vTaskResume(threadStruct.tm4c123ThreadId);
            error = ErrorType::Success;
            break;
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