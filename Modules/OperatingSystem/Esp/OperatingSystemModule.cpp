//Modules
#include "OperatingSystemModule.hpp"
#include "Math.hpp"
#include "ProcessorModule.hpp"
//Posix
#include <pthread.h>
#include <unistd.h>
//C++
#include <cstring>
//ESP
#include "esp_pthread.h"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(TimerHandle_t timer);

#ifdef __cplusplus
}
#endif

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    //https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/pthread.html#rtos-integration
    //See Note. If the delay is less than the tick period then the new thread will never block. At startup this is a
    //big problem because the main thread will always be lower priority and never be scheduled again as soon as we
    //create the first thread so if the first thread blocks waiting for other threads it will starve the main thread
    //and it will not be able to create the other threads.

    //The default tick rate is 100Hz, so trying to delay for a thousandth of a second is 10 times shorter than the
    //minimum needed to block.
    #if configTICK_RATE_HZ < 1000
        #warning "If you have delays of 1ms or less the block time will be increased to ensure FreeRTOS actually blocks the task."
    #endif
    const Milliseconds minimumDelayToBlock = delay * (1000 / configTICK_RATE_HZ);
    usleep(minimumDelayToBlock * 1000);
    return ErrorType::Success;
}

ErrorType OperatingSystem::delay(const Microseconds delay) {
    if (configTICK_RATE_HZ <= 1000) {
        return ErrorType::NotSupported;
    }

    const TickType_t now = xTaskGetTickCount();
    const TickType_t oneMillisecond = pdMS_TO_TICKS(1);
    const Microseconds oneMicrosecond = oneMillisecond * 1000;

    Microseconds remainingDelay = delay;

    if (oneMicrosecond > 0) {
        while ((differenceBetween(now, xTaskGetTickCount()) / oneMicrosecond) < remainingDelay);
    }

    return ErrorType::Success;
}


//ESP will handle starting FreeRTOS for you by the time you get to app_main
ErrorType OperatingSystem::startScheduler() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::createThread(const OperatingSystemTypes::Priority priority, const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    ErrorType error = ErrorType::Failure;
    static Id nextThreadId = 1;

    //On ESP, the start function is called before pthread_create returns so we have to add in an init function to make sure
    //that the details of thread are properly saved before the thread code runs. For example, if a thread calls currentThreadId,
    //the posix ID will not be saved yet because pthread_create has not returned and so this function will fail even though the thread
    //exists and has an ID.
    struct InitThreadArgs {
        void *arguments;
        void *(*startFunction)(void *);
        TaskHandle_t *threadId;
    };
    auto initThread = [](void *arguments) -> void {
        InitThreadArgs *initThreadArgs = static_cast<InitThreadArgs *>(arguments);
        *(initThreadArgs->threadId) = xTaskGetCurrentTaskHandle();
        (initThreadArgs->startFunction)(initThreadArgs->arguments);
        delete initThreadArgs;
        return;
    };

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

    InitThreadArgs *initThreadArgs = new InitThreadArgs {
        .arguments = arguments,
        .startFunction = startFunction,
        .threadId = &threads[name].espThreadId,
    };

    TaskHandle_t thread;
    const bool threadWasCreated = (pdPASS == xTaskCreate(initThread, name.data(), stackSize/4, initThreadArgs, toEspPriority(priority), &thread));
    if (threadWasCreated) {
        error = ErrorType::Success;
    }
    else {
        deleteThread(name);
        return ErrorType::Failure;
    }

    _status.threadCount = threads.size();

    return error;
}

ErrorType OperatingSystem::deleteThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    _status.threadCount = threads.size();

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

    auto it = std::find_if(threads.begin(), threads.end(), [threadId](const auto &pair) { return pair.second.espThreadId == threadId; });
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

    semaphores[name] = freertosSemaphore;

    return ErrorType::Success;
}

ErrorType OperatingSystem::deleteSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    vSemaphoreDelete(semaphores[name]);

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
    TimerHandle_t timerHandle;
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
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::sendToQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::receiveFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::peekFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
    return ErrorType::NotImplemented;
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
    //app_get_description is in the format of "git describe --tag". So will stop when it encounters a '-'
    //https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32/api-reference/system/misc_system_api.html
    const esp_app_desc_t *appDescription = esp_app_get_description();

    for (size_t i = 0; i < strlen(appDescription->version) && appDescription->version[i] != '-'; i++) {
        if (appDescription->version[i] == '.') {
            softwareVersion.push_back('.');
            continue;
        }

        softwareVersion.push_back(appDescription->version[i]);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::getResetReason(OperatingSystemTypes::ResetReason &resetReason) {
    ErrorType error;
    resetReason = toPlatformResetReason(esp_reset_reason(), error);
    return error;
}

ErrorType OperatingSystem::reset() {
    esp_restart();
    return ErrorType::Success;
}

ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) {
    struct timeval tv;
    tv.tv_sec = utc + timeZoneDifferenceUtc;
    tv.tv_usec = 0;
    return fromPlatformError(settimeofday(&tv, nullptr));
}

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
#if configUSE_IDLE_HOOK != 1
#warning "configUSE_IDLE_HOOK is not enabled. Idle percentage will not be updated."
#endif
    _status.idle = idlePercent;
    return ErrorType::Success;
}

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) {
    constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> dram = {"DRAM"};
    constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> spiram = {"SPIRAM"};

    if (memoryRegionName.empty()) {
        return ErrorType::InvalidParameter;
    }
    else if (0 == strncmp(memoryRegionName.data(), dram.data(), OperatingSystemTypes::MaxMemoryRegionNameLength)) {
        size = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    }
    else if (0 == strncmp(memoryRegionName.data(), spiram.data(), OperatingSystemTypes::MaxMemoryRegionNameLength)) {
        size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) {
    constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> dram = {"DRAM"};
    constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> spiram = {"SPIRAM"};

    if (memoryRegionName.empty()) {
        return ErrorType::InvalidParameter;
    }
    else if (0 == strncmp(memoryRegionName.data(), dram.data(), OperatingSystemTypes::MaxMemoryRegionNameLength)) {
        size = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    }
    else if (0 == strncmp(memoryRegionName.data(), spiram.data(), OperatingSystemTypes::MaxMemoryRegionNameLength)) {
        size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    }

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

//https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html#critical-sections
//These are specific to ESP_IDF version of FreeRTOS
ErrorType OperatingSystem::disableAllInterrupts() {
    if (ErrorType::Success == Processor::Instance().isInterruptContext()) {
        taskENTER_CRITICAL_ISR(&_interruptSpinlock);
    }
    else {
        taskENTER_CRITICAL(&_interruptSpinlock);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::enableAllInterrupts() {
    if (ErrorType::Success == Processor::Instance().isInterruptContext()) {
        taskEXIT_CRITICAL_ISR(&_interruptSpinlock);
    }
    else {
        taskEXIT_CRITICAL(&_interruptSpinlock);
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
    if (OperatingSystem::IsInitialized()) {
        OperatingSystem::Instance().idlePercentage(idle);
    }

    numberOfTimesIdleHookHasBeenCalled++;

    timeOfLastIdleHookCall = timeNow;
}

#ifdef __cplusplus
}
#endif
