//Modules
#include "OperatingSystemModule.hpp"
//C++
#include <ctime>
//Posix
#include <unistd.h>
#include <pthread.h>
#include <sys/times.h>

ErrorType OperatingSystem::delay(Milliseconds delay) {
    vTaskDelay(delay);
    return ErrorType::Success;
}

ErrorType OperatingSystem::createThread(OperatingSystemConfig::Priority priority, std::string name, void * arguments, Bytes stackSize, void *(*startFunction)(void *), Id &number) {
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
        return toPlatformError(retc);
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
        error = toPlatformError(retc);
    }

    _status.threadCount = threads.size();

    return error;
}

//I want to use pthreads since I like the portability of them, however, ESP does not implement pthread_kill.
//The work around is to set the thread in the deatched state and then have the main loops of each thread regularly check their status
//to see if they have been terminated by the operating system, which will set isTerminated when the thread is detached.
ErrorType OperatingSystem::deleteThread(std::string name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    return error;
}

ErrorType OperatingSystem::joinThread(std::string name) {
    Id thread;
    int ret;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    ret = pthread_join(threads[name].cc32xxThreadId, nullptr);
    return toPlatformError(ret);
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
    //The internal name is the name with a leading / to make it a valid semaphore name on POSIX systems.
    //For all other purposes inside this operating system abstraction, the name should be used directly.
    std::string internalName = std::string("/").append(name);

    sem_t *semaphore = nullptr;
    int pshared = 0;
    int ret = sem_init(semaphore, pshared, initial);
    if (0 == ret) {
        return toPlatformError(ret);
    }
    else {
        semaphores[name] = semaphore;
        return ErrorType::Success;
    }
}

ErrorType OperatingSystem::deleteSemaphore(std::string name) {
    std::string internalName = std::string("/").append(name);

    if (0 != sem_destroy(semaphores[name])) {
        return toPlatformError(errno);
    }

    semaphores.erase(name);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(std::string &name, Milliseconds timeout) {
    Milliseconds timeRemaining = timeout;
    constexpr Milliseconds delayTime = 1;
    int result;

    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::string internalName = std::string("/").append(name);

    do {
        if (0 != (result = sem_wait(semaphores[name]))) {
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
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::incrementSemaphore(std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::string internalName = std::string("/").append(name);

    if (0 != sem_post(semaphores[name])) {
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(std::string name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::string internalName = std::string("/").append(name);

    if (0 != sem_trywait(semaphores[name])) {
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::startTimer(Id timer, Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::stopTimer(Id timer, Milliseconds timeout) {
    return ErrorType::NotImplemented;
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

ErrorType OperatingSystem::ticksToMilliseconds(Ticks ticks, Milliseconds &timeInMilliseconds) {
    timeInMilliseconds = static_cast<Milliseconds>(ticks * sysconf(_SC_CLK_TCK) / 1000);
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
ErrorType OperatingSystem::setTimeOfDay(UnixTime utc, Seconds timeZoneDifferenceUtc) {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
    return ErrorType::NotImplemented;
}