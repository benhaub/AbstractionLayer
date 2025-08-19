//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "Math.hpp"
#include "ProcessorModule.hpp"
//C++
#include <ctime>
//Posix
#include <unistd.h>
#include <pthread.h>
#include <sys/times.h>
#include <signal.h> //For timers

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
    pthread_attr_t attrs;
    struct sched_param priParam;
    int retc;
    ErrorType error = ErrorType::Failure;
    static Id nextThreadId = 1;

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = toCc32xxPriority(priority);
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, stackSize);
    if (retc != 0) {
        return fromPlatformError(retc);
    }

    Thread newThread = {
        .name = name,
        .threadId = nextThreadId++
    };

    if (threads.size() < _MaxThreads) {
        threads[name] = newThread;
    }
    else {
        return ErrorType::LimitReached;
    }

    number = newThread.threadId;

    pthread_t thread;
    const bool threadWasCreated = (0 == (retc = pthread_create(&thread, &attrs, startFunction, arguments)));
    if (threadWasCreated) {
        error = ErrorType::Success;
        threads[name].cc32xxThreadId = thread;
    }
    else {
        deleteThread(name);
        error = fromPlatformError(retc);
    }

    _status.threadCount = threads.size();

    return error;
}

//I want to use pthreads since I like the portability of them, however, ESP does not implement pthread_kill.
//The work around is to set the thread in the deatched state and then have the main loops of each thread regularly check their status
//to see if they have been terminated by the operating system, which will set isTerminated when the thread is detached.
ErrorType OperatingSystem::deleteThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    return error;
}

ErrorType OperatingSystem::joinThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    Id thread;
    int ret;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    ret = pthread_join(threads[name].cc32xxThreadId, nullptr);
    return fromPlatformError(ret);
}

ErrorType OperatingSystem::threadId(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, Id &thread) {
    if (threads.contains(name)) {
        thread = threads[name].threadId;
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::currentThreadId(Id &thread) const {
    pthread_t task = pthread_self();
    auto it = std::find_if(threads.begin(), threads.end(), [task](const auto &pair) { return pair.second.cc32xxThreadId == task; });
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
    sem_t semaphore;
    int pshared = 0;
    int ret = sem_init(&semaphore, pshared, initial);
    if (-1 == ret) {
        return fromPlatformError(errno);
    }
    else {
        semaphores[name] = semaphore;
        return ErrorType::Success;
    }
}

ErrorType OperatingSystem::deleteSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (0 != sem_destroy(&(semaphores[name]))) {
        return fromPlatformError(errno);
    }

    semaphores.erase(name);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name, const Milliseconds timeout) {
    Milliseconds timeRemaining = timeout;
    constexpr Milliseconds delayTime = 1;
    int result;

    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    do {
        if (0 != (result = sem_wait(&(semaphores[name])))) {
            if (timeRemaining > 0) {
                delay(delayTime);
            }
            timeRemaining >= delayTime ? timeRemaining -= delayTime : timeRemaining = 0;
            if (0 == timeRemaining) {
                return ErrorType::Timeout;
            }
        }
    } while (EAGAIN == errno);


    if (0 != result) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::incrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_post(&(semaphores[name]))) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_trywait(&(semaphores[name]))) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
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

ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) {
    struct timespec timeSpec;
    timeSpec.tv_sec = utc + timeZoneDifferenceUtc;
    timeSpec.tv_nsec = 0;

    return fromPlatformError(clock_settime(CLOCK_REALTIME, &timeSpec));
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