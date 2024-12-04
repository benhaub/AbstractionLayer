//Modules
#include "OperatingSystemModule.hpp"
//C++
#include <ctime>
//Posix
#include <sys/times.h>
#include <sys/time.h>
#include <sys/syslimits.h>

ErrorType OperatingSystem::delay(Milliseconds delay) {
    usleep(delay*1000);
    return ErrorType::Success;
}

ErrorType OperatingSystem::createThread(OperatingSystemConfig::Priority priority, std::string name, void * arguments, Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    pthread_attr_t attr;
    sched_param param;
    int res;
    pthread_t thread;
    static Id nextThreadId = 1;

    res = pthread_attr_init(&attr);
    assert(0 == res);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, stackSize);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = toPosixPriority(priority);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    const bool threadWasCreated = (0 == (res = pthread_create(&thread, &attr, startFunction, arguments)));
    pthread_attr_destroy(&attr);
    if (threadWasCreated) {
        Thread newThread = {
            .posixThreadId = thread,
            .name = name,
            .threadId = nextThreadId++
        };

        number = newThread.threadId;

        if (threads.size() < MaxThreads) {
            threads[name] = newThread;
            return ErrorType::Success;
        }
        else {
            deleteThread(name);
            return ErrorType::LimitReached;
        }
    }
    else {
        return toPlatformError(res);
    }
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
    //The internal name is the name with a leading / to make it a valid semaphore name on POSIX systems.
    //For all other purposes inside this operating system abstraction, the name should be used directly.
    std::string internalName = std::string("/").append(name);

    if (internalName.size() > NAME_MAX-4) {
        return ErrorType::InvalidParameter;
    }

    //On POSIX systems, a created semaphore has peristence within the kernel until it is removed.
    //So delete old semaphores first and then create the new one since we specify O_EXCL as a flag.
    deleteSemaphore(name); //Using name and not internalName is NOT a bug.
    sem_t *semaphore = sem_open(internalName.c_str(), O_CREAT | O_EXCL, S_IRWXU, initial);
    if (SEM_FAILED == semaphore) {
        return toPlatformError(errno);
    }
    else {
        semaphores[name] = semaphore;
        return ErrorType::Success;
    }
}

ErrorType OperatingSystem::deleteSemaphore(std::string name) {
    std::string internalName = std::string("/").append(name);

    if (0 != sem_unlink(internalName.c_str())) {
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

ErrorType OperatingSystem::deleteTimer(const Id timer) {
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
    std::string softwareVersionStringRaw(32, 0);
    
    constexpr char command[] = "sh -c \"git describe --tag\"";
    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        if (nullptr != fgets(softwareVersionStringRaw.data(), softwareVersionStringRaw.capacity(), pipe)) {
            error = ErrorType::Success;
        }
        else {
            softwareVersionStringRaw.clear();
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    for (unsigned int i = 0; i < softwareVersionStringRaw.size() && softwareVersionStringRaw.at(i) != '-'; i++) {
        if (softwareVersionStringRaw.at(i) == '.') {
            softwareVersion.push_back('.');
            continue;
        }

        softwareVersion.push_back(softwareVersionStringRaw.at(i));
    }

    return error;
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

ErrorType OperatingSystem::pid(Id &pid) {
    ErrorType error = ErrorType::Failure;
    std::string pidString(16, 0);

    const std::string commandPid("pgrep -i foundation | tail -1");

    FILE *pipe = popen(commandPid.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(pidString.data(), sizeof(uint8_t), pidString.capacity(), pipe);
        if (feof(pipe) || bytesRead == pidString.capacity()) {
            error = ErrorType::Success;
            pidString.resize(bytesRead);
            while (pidString.back() == '\n') {
                pidString.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    pid = strtoul(pidString.c_str(), nullptr, 10);

    return error;
}

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
    ErrorType error = ErrorType::Failure;
    std::string cpuUtilization(16, 0);
    Percent cpuUtilizationPercent;
    Id processId;
    pid(processId);

    const std::string commandPercentUtilization(std::string("ps -p ").append(std::to_string(processId)).append(" -o %cpu | tail -1"));

    FILE *pipe = popen(commandPercentUtilization.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(cpuUtilization.data(), sizeof(uint8_t), cpuUtilization.capacity(), pipe);
        if (feof(pipe) || bytesRead == cpuUtilization.capacity()) {
            error = ErrorType::Success;
            cpuUtilization.resize(bytesRead);
            while (cpuUtilization.back() == '\n') {
                cpuUtilization.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    cpuUtilizationPercent = strtof(cpuUtilization.c_str(), nullptr);
    idlePercent = 100.0f - cpuUtilizationPercent;

    return error;
}
