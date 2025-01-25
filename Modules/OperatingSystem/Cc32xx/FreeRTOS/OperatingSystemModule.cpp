//Modules
#include "OperatingSystemModule.hpp"
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

void TimerCallback(union sigval val);

#ifdef __cplusplus
}
#endif

ErrorType OperatingSystem::delay(Milliseconds delay) {
    vTaskDelay(delay);
    return ErrorType::Success;
}

ErrorType OperatingSystem::startScheduler() {
    vTaskStartScheduler();

    //Never returns
    return ErrorType::Failure;
}

ErrorType OperatingSystem::createThread(const OperatingSystemConfig::Priority priority, const std::string &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    pthread_t thread;
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

    //On cc32xx, the start function is called before pthread_create returns so we have to add our thread before we create it.
    Thread newThread = {
        .cc32xxThreadId = thread,
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
ErrorType OperatingSystem::deleteThread(const std::string &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    return error;
}

ErrorType OperatingSystem::joinThread(const std::string &name) {
    Id thread;
    int ret;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    ret = pthread_join(threads[name].cc32xxThreadId, nullptr);
    return fromPlatformError(ret);
}

ErrorType OperatingSystem::threadId(const std::string &name, Id &thread) {
    if (threads.contains(name)) {
        thread = threads[name].threadId;
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::isDeleted(const std::string &name) {
    if (threads.contains(name)) {
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::createSemaphore(const Count max, const Count initial, const std::string &name) {
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

ErrorType OperatingSystem::deleteSemaphore(const std::string &name) {
    if (0 != sem_destroy(&(semaphores[name]))) {
        return fromPlatformError(errno);
    }

    semaphores.erase(name);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(const std::string &name, const Milliseconds timeout) {
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

ErrorType OperatingSystem::incrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_post(&(semaphores[name]))) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_trywait(&(semaphores[name]))) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::createTimer(Id &timer, const Milliseconds period, const bool autoReload, std::function<void(void)> callback) {
    timer_t timerId;
    Timer newTimer = {
        .callback = callback,
        .id = nextTimerId++,
        .autoReload = autoReload,
        .period = period
    };
    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_value.sival_ptr = reinterpret_cast<void *>(timerId);
    sev.sigev_notify_function = TimerCallback;
    sev.sigev_notify_attributes = nullptr;

    const bool timerCreated = 0 == timer_create(CLOCK_MONOTONIC, &sev, &timerId);

    if (timerCreated) {
        timers[timerId] = newTimer;
        timer = newTimer.id;
        return ErrorType::Success;
    }

    //Make sure the timer does not start.
    constexpr uint8_t stopTimer = 0;
    its.it_value.tv_nsec = its.it_value.tv_sec = stopTimer;

    //Start the timer
    if (autoReload) {
        //If the interval specifies a non-zero value (either field) then is will fire a time continuously on the period.
        its.it_interval.tv_sec = its.it_interval.tv_nsec = period * 1E6;
    }
    else {
        constexpr uint8_t doNotAutoReload = 0;
        //For a single shot timer, the interval value is zero (both values are zero)
        its.it_interval.tv_sec = its.it_interval.tv_nsec = doNotAutoReload;
    }

    constexpr int flags = 0;
    int res = timer_settime(timerId, flags, &its, nullptr);

    if (0 != res) {
        deleteTimer(timer);
        return fromPlatformError(res);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::deleteTimer(const Id timer) {
    auto itr = timers.begin();

    while (itr != timers.end()) {
        if (timer == itr->second.id) {
            timer_delete(itr->first);
            timers.erase(itr);
            return ErrorType::Success;
        }
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::startTimer(const Id timer, const Milliseconds timeout) {
    auto itr = timers.begin();
    struct itimerspec *timerspec = nullptr;

    while (itr != timers.end()) {
        if (timer == itr->second.id) {
            timer_gettime(itr->first, timerspec);
            //For auto reload timers, the it_value needs to be non-zero (either field) for the timer to start
            //timing according to the interval field. For single shot timers, the it_value field should be the value
            //that the timer will count down from.
            assert(nullptr != timerspec);
            timerspec->it_value.tv_nsec = itr->second.period * 1E6;
            return ErrorType::Success;
        }
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::stopTimer(const Id timer, const Milliseconds timeout) {
    auto itr = timers.begin();
    struct itimerspec *timerspec = nullptr;

    while (itr != timers.end()) {
        if (timer == itr->second.id) {
            timer_gettime(itr->first, timerspec);
            //For auto reload timers, the it_value needs to be non-zero (either field) for the timer to start
            //timing according to the interval field. For single shot timers, the it_value field should be the value
            //that the timer will count down from.
            assert(nullptr != timerspec);
            timerspec->it_value.tv_nsec = timerspec->it_value.tv_sec = 0;
            return ErrorType::Success;
        }
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::createQueue(const std::string &name, const Bytes size, const Count length) {
    QueueHandle_t handle = nullptr;

    handle = xQueueCreate(length, size);

    if (nullptr == handle) {
        return ErrorType::NoMemory;
    }

    queues[name] = handle;

    return ErrorType::Success;
}

ErrorType OperatingSystem::sendToQueue(const std::string &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) {
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

ErrorType OperatingSystem::receiveFromQueue(const std::string &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
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

ErrorType OperatingSystem::peekFromQueue(const std::string &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
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

ErrorType OperatingSystem::getSystemTick(Ticks &currentSystemTicks) {
    struct tms timeSample;
    currentSystemTicks = static_cast<Ticks>(times(&timeSample));
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

ErrorType OperatingSystem::getResetReason(OperatingSystemConfig::ResetReason &resetReason) {
    //There isn't really such thing as a reset reason for most posix systems, so we'll just call it power-on.
    resetReason = OperatingSystemConfig::ResetReason::PowerOn;
    return ErrorType::Success;
}

ErrorType OperatingSystem::reset() {
    return ErrorType::NotAvailable;
}

//On system that use Posix, you shouldn't attempt to set the time of day, and the time that can be obtained
//using the posix API will already be the correct time that you need as soon as you start your application.
ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::string &memoryRegionName) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::string &memoryRegionName) {
    return ErrorType::NotImplemented;
}

void OperatingSystem::callTimerCallback(timer_t timer) {
    timers[timer].callback();

    if (timers[timer].autoReload) {
        deleteTimer(timers[timer].id);
    }

    return;
}

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(union sigval val) {
    //Only the value of the pointer matters. Not what it pointer to.
    //You do not need to worry about keeping the timer_t memory in scope.
    timer_t timer = reinterpret_cast<timer_t>(val.sival_ptr);
    OperatingSystem::Instance().callTimerCallback(timer);
}

#ifdef __cplusplus
}
#endif