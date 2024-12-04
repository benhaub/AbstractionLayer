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

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(TimerHandle_t timer);

#ifdef __cplusplus
}
#endif

ErrorType OperatingSystem::delay(Milliseconds delay) {
    usleep(delay * 1000);
    return ErrorType::Success;
}

ErrorType OperatingSystem::createThread(OperatingSystemConfig::Priority priority, std::string name, void * arguments, Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    esp_pthread_cfg_t esp_pthread_cfg;
    pthread_t thread;
    int res;
    ErrorType error = ErrorType::Failure;
    static Id nextThreadId = 1;

    //Hopefully these are joinable by default, because it seems like it really messes stuff up to use
    //pthread config and esp_pthread config at the same time.
    esp_pthread_cfg = esp_pthread_get_default_config();
    esp_pthread_cfg.stack_size = stackSize;
    esp_pthread_cfg.prio = toEspPriority(priority);
    esp_pthread_cfg.thread_name = name.c_str();

    if (ErrorType::Success != (error = toPlatformError(esp_pthread_set_cfg(&esp_pthread_cfg)))) {
        return error;
    }

    //On ESP, the start function is called before pthread_create returns so we have to add our thread before we create it.
    Thread newThread = {
        .posixThreadId = thread,
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

    const bool threadWasCreated = (0 == (res = pthread_create(&thread, NULL, startFunction, arguments)));
    if (threadWasCreated) {
        error = ErrorType::Success;
        threads[name].posixThreadId = thread;
    }
    else {
        deleteThread(name);
        error = toPlatformError(res);
    }

    _status.threadCount = threads.size();

    return error;
}

ErrorType OperatingSystem::deleteThread(std::string name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    _status.threadCount = threads.size();

    return error;
}

ErrorType OperatingSystem::joinThread(std::string name) {
    Id thread;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    return toPlatformError(pthread_join(threads[name].posixThreadId, nullptr));
}

ErrorType OperatingSystem::threadId(std::string name, Id &thread) {
    if (threads.contains(name)) {
        thread = threads[name].threadId;
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::isDeleted(std::string &name) {
    if (threads.contains(name)) {
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::createSemaphore(Count max, Count initial, std::string name) {
    SemaphoreHandle_t freertosSemaphore;
    freertosSemaphore = xSemaphoreCreateCounting(max, initial);

    semaphores[name] = freertosSemaphore;

    return ErrorType::Success;
}

ErrorType OperatingSystem::deleteSemaphore(std::string name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    vSemaphoreDelete(semaphores[name]);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(std::string &name, Milliseconds timeout) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreTake(semaphores[name], timeout)) {
        return ErrorType::Success;
    }

    return ErrorType::Timeout;
}

ErrorType OperatingSystem::incrementSemaphore(std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreGive(semaphores[name])) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::decrementSemaphore(std::string name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (pdTRUE == xSemaphoreTake(semaphores[name], 0)) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType OperatingSystem::createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) {
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

ErrorType OperatingSystem::startTimer(Id timer, Milliseconds timeout) {
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

ErrorType OperatingSystem::stopTimer(Id timer, Milliseconds timeout) {
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

ErrorType OperatingSystem::getSystemTime(UnixTime &currentSystemUnixTime) {
    currentSystemUnixTime = static_cast<UnixTime>(time(nullptr));
    return ErrorType::Success;
}

ErrorType OperatingSystem::getSystemTick(Ticks &currentSystemTick) {
    currentSystemTick = static_cast<Ticks>(xTaskGetTickCount());
    return ErrorType::Success;
}

ErrorType OperatingSystem::ticksToMilliseconds(Ticks ticks, Milliseconds &timeInMilliseconds) {
    timeInMilliseconds = static_cast<Milliseconds>(pdMS_TO_TICKS(ticks));
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
    resetReason = toCbtResetReason(esp_reset_reason(), error);
    return error;
}

ErrorType OperatingSystem::reset() {
    esp_restart();
    return ErrorType::Success;
}

ErrorType OperatingSystem::setTimeOfDay(UnixTime utc, Seconds timeZoneDifferenceUtc) {
    struct timeval tv;
    tv.tv_sec = utc + timeZoneDifferenceUtc;
    tv.tv_usec = 0;
    return toPlatformError(settimeofday(&tv, nullptr));
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

void OperatingSystem::callTimerCallback(TimerHandle_t timer) {
    timers[timer].callback();

    if (timers[timer].autoReload) {
        deleteTimer(timers[timer].id)
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
