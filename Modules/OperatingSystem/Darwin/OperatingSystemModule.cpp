//Modules
#include "OperatingSystemModule.hpp"
//C++
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cstdio>
//Posix
#include <sys/times.h>
#include <sys/time.h>
#include <sys/syslimits.h>

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    usleep(delay*1000);
    return ErrorType::Success;
}


ErrorType OperatingSystem::delay(Microseconds delay) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::startScheduler() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::createThread(const OperatingSystemConfig::Priority priority, const std::string &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    pthread_attr_t attr;
    sched_param param;
    int res;
    pthread_t thread;
    static Id nextThreadId = 1;

    res = pthread_attr_init(&attr);
    assert(0 == res);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
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

ErrorType OperatingSystem::deleteThread(const std::string &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    return error;
}

ErrorType OperatingSystem::joinThread(const std::string &name) {
    Id thread;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    return toPlatformError(pthread_join(threads[name].posixThreadId, nullptr));
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

ErrorType OperatingSystem::deleteSemaphore(const std::string &name) {
    std::string internalName = std::string("/").append(name);

    if (0 != sem_unlink(internalName.c_str())) {
        return toPlatformError(errno);
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

ErrorType OperatingSystem::incrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_post(semaphores[name])) {
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_trywait(semaphores[name])) {
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::createTimer(Id &timer, const Milliseconds period, const bool autoReload, std::function<void(void)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::deleteTimer(const Id timer) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::startTimer(const Id timer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::stopTimer(const Id timer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
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

ErrorType OperatingSystem::getSystemTick(Ticks &currentSystemTicks) {
    struct tms timeSample;
    currentSystemTicks = static_cast<Ticks>(times(&timeSample));
    return ErrorType::Success;
}

ErrorType OperatingSystem::ticksToMilliseconds(const Ticks ticks, Milliseconds &timeInMilliseconds) {
    timeInMilliseconds = static_cast<Milliseconds>(ticks * sysconf(_SC_CLK_TCK) / 1000);
    return ErrorType::Success;
}

ErrorType OperatingSystem::millisecondsToTicks(const Milliseconds milli, Ticks &ticks) {
    return ErrorType::NotImplemented;
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
ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::pid(Id &pid) {
    ErrorType error = ErrorType::Failure;
    std::string pidString(16, 0);

    std::string programName(getprogname(), strlen(getprogname()));
    std::string commandPid("pgrep -if ");
    commandPid.append(programName);
    commandPid.append(" | tail -1");

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

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::string &memoryRegionName) {
    ErrorType error = ErrorType::Failure;

    //Will return the size of RAM in GB.
    std::string commandFinal("system_profiler SPMemoryDataType | egrep Memory | tail -2 | tr -s \" \" | cut -d \" \" -f3 | tail -1");
    std::string ramSize(4, 0);
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(ramSize.data(), sizeof(uint8_t), ramSize.capacity(), pipe);
        if (feof(pipe) || bytesRead == ramSize.capacity()) {
            error = ErrorType::Success;
            ramSize.resize(bytesRead);
            while (ramSize.back() == '\n') {
                ramSize.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }
    size = std::strtoul(ramSize.c_str(), nullptr, 10);
    size = size * 1024 * 1024;

    return error;
}

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::string &memoryRegionName) {
    ErrorType error = ErrorType::Failure;

    //Will return the size of RAM used in kilobytes
    std::string commandFinal("ps -o rss | awk '{sum += $1} END {print sum}'");
    std::string ramUsed(6, 0);
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(ramUsed.data(), sizeof(uint8_t), ramUsed.capacity(), pipe);
        if (feof(pipe) || bytesRead == ramUsed.capacity()) {
            error = ErrorType::Success;
            ramUsed.resize(bytesRead);
            while (ramUsed.back() == '\n') {
                ramUsed.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    Kilobytes totalRam;
    maxHeapSize(totalRam);

    size = totalRam - std::strtoul(ramUsed.c_str(), nullptr, 10);

    return error;
}

ErrorType OperatingSystem::uptime(Seconds &uptime) {
    return ErrorType::NotImplemented;
}