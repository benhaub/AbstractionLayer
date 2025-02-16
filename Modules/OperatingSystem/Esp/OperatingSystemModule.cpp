//Modules
#include "OperatingSystemModule.hpp"
//Posix
#include <pthread.h>
#include <unistd.h>
//C++
#include <cstring>
//ESP
#include "esp_pthread.h"
#include "esp_app_desc.h"
#include "esp_log.h"
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
    const Milliseconds minimumDelayToBlock = delay * (1000 / configTICK_RATE_HZ);
    usleep(minimumDelayToBlock * 1000);
    return ErrorType::Success;
}

//ESP will handle starting FreeRTOS for you by the time you get to app_main
ErrorType OperatingSystem::startScheduler() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::createThread(const OperatingSystemConfig::Priority priority, const std::string &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    esp_pthread_cfg_t esp_pthread_cfg;
    int res;
    ErrorType error = ErrorType::Failure;
    static Id nextThreadId = 1;

    esp_pthread_cfg = esp_pthread_get_default_config();
    esp_pthread_cfg.stack_size = stackSize;
    esp_pthread_cfg.prio = toEspPriority(priority);
    esp_pthread_cfg.thread_name = name.c_str();

    if (ErrorType::Success != (error = fromPlatformError(esp_pthread_set_cfg(&esp_pthread_cfg)))) {
        return error;
    }

    //On ESP, the start function is called before pthread_create returns so we have to add in an init function to make sure
    //that the details of thread are properly saved before the thread code runs. For example, if a thread calls currentThreadId,
    //the posix ID will not be saved yet because pthread_create has not returned and so this function will fail even though the thread
    //exists and has an ID.
    struct InitThreadArgs {
        void *arguments;
        void *(*startFunction)(void *);
        pthread_t *threadId;
    };
    auto initThread = [](void *arguments) -> void * {

        InitThreadArgs *initThreadArgs = static_cast<InitThreadArgs *>(arguments);
        *(initThreadArgs->threadId) = pthread_self();
        return (initThreadArgs->startFunction)(initThreadArgs->arguments);
    };

    Thread newThread = {
        .name = name,
        .threadId = nextThreadId++
    };

    if (threads.size() < MaxThreads) {
        threads[name] = newThread;
    }
    else {
        return ErrorType::LimitReached;
    }

    number = newThread.threadId;

    InitThreadArgs initThreadArgs = {
        .arguments = arguments,
        .startFunction = startFunction,
        .threadId = &threads[name].posixThreadId,
    };

    //thread parameter can not be NULL.
    pthread_t thread;
    const bool threadWasCreated = (0 == (res = pthread_create(&thread, NULL, initThread, &initThreadArgs)));
    if (threadWasCreated) {
        error = ErrorType::Success;
    }
    else {
        deleteThread(name);
        error = fromPlatformError(res);
    }

    _status.threadCount = threads.size();

    return error;
}

ErrorType OperatingSystem::deleteThread(const std::string &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    _status.threadCount = threads.size();

    return error;
}

ErrorType OperatingSystem::joinThread(const std::string &name) {
    Id thread;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    return fromPlatformError(pthread_join(threads[name].posixThreadId, nullptr));
}

ErrorType OperatingSystem::threadId(const std::string &name, Id &thread) {
    if (threads.contains(name)) {
        thread = threads[name].threadId;
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::currentThreadId(Id &thread) const {
    pthread_t task = pthread_self();
    auto it = std::find_if(threads.begin(), threads.end(), [task](const auto &pair) { return pair.second.posixThreadId == task; });
    if (threads.end() == it) {
        return ErrorType::NoData;
    }

    thread = it->second.threadId;

    return ErrorType::Success;
}

ErrorType OperatingSystem::isDeleted(const std::string &name) {
    if (threads.contains(name)) {
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::createSemaphore(const Count max, const Count initial, const std::string &name) {
    SemaphoreHandle_t freertosSemaphore;
    freertosSemaphore = xSemaphoreCreateCounting(max, initial);

    semaphores[name] = freertosSemaphore;

    return ErrorType::Success;
}

ErrorType OperatingSystem::deleteSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    vSemaphoreDelete(semaphores[name]);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(const std::string &name, const Milliseconds timeout) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreTake(semaphores[name], timeout)) {
        return ErrorType::Success;
    }

    return ErrorType::Timeout;
}

ErrorType OperatingSystem::incrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreGive(semaphores[name])) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::decrementSemaphore(const std::string &name) {
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

ErrorType OperatingSystem::createQueue(const std::string &name, const Bytes size, const Count length) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::sendToQueue(const std::string &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::receiveFromQueue(const std::string &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::peekFromQueue(const std::string &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
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

ErrorType OperatingSystem::getResetReason(OperatingSystemConfig::ResetReason &resetReason) {
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
    static time_t runtime_total = 0;
    static uint64_t idle_task_runtime_last = 0;

    // get the time between calls
    time_t runtime_total_now = time(NULL);
    float runtime_between_calls = (float)(runtime_total_now - runtime_total);
    runtime_total = runtime_total_now;
    // get the idle time between calls
    uint32_t idle_time_between_calls;
    //You have to enable run time statistics for this to compile.
    uint32_t idle_runtime = ulTaskGetIdleRunTimeCounter() * 1E-6;
    if (idle_runtime < idle_task_runtime_last) {
    idle_time_between_calls = idle_task_runtime_last - idle_runtime;
    }
    else {
    idle_time_between_calls = idle_runtime - idle_task_runtime_last;
    }
    idle_task_runtime_last = idle_runtime;

    idlePercent = 100.0f * (float)idle_time_between_calls / runtime_between_calls;

    return ErrorType::Success;
}

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::string &memoryRegionName) {
    if (0 == memoryRegionName.compare(0, sizeof("DRAM")-1, "DRAM")) {
        size = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    }
    else if (0 == memoryRegionName.compare(0, sizeof("SPIRAM")-1, "SPIRAM")) {
        size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::string &memoryRegionName) {
    if (0 == memoryRegionName.compare(0, sizeof("DRAM")-1, "DRAM")) {
        size = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    }
    else if (0 == memoryRegionName.compare(0, sizeof("SPIRAM")-1, "SPIRAM")) {
        size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
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

#ifdef __cplusplus
}
#endif
