//Modules
#include "OperatingSystemModule.hpp"
//C++
#include <ctime>
#include <cstdio>
//C
#include <string.h>
//Posix
#include <sys/times.h>
#include <sys/time.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <sys/stat.h>

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    usleep(delay*1000);
    return ErrorType::Success;
}

ErrorType OperatingSystem::startScheduler() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::createThread(const OperatingSystemConfig::Priority priority, const std::string &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    pthread_attr_t attr;
    sched_param param;
    int res;
    static Id nextThreadId = 1;
    ErrorType error = ErrorType::Failure;

    res = pthread_attr_init(&attr);
    assert(0 == res);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, stackSize);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = toPosixPriority(priority);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    //On Linux, the start function is called before pthread_create returns so we have to add in an init function to make sure
    //that the details of thread are properly saved before the thread code runs. For example, if a thread calls currentThreadId,
    //the posix ID will not be saved yet because pthread_create has not returned and so this function will fail even though the thread
    //exists and has an ID.
    struct InitThreadArgs {
        void *arguments = nullptr;
        void *(*startFunction)(void *);
        pthread_t *threadId = nullptr;
    };
    auto initThread = [](void *arguments) -> void * {

        InitThreadArgs *initThreadArgs = static_cast<InitThreadArgs *>(arguments);
        *(initThreadArgs->threadId) = pthread_self();
        (initThreadArgs->startFunction)(initThreadArgs->arguments);
        delete initThreadArgs;
        return nullptr;
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
    threads[name].posixThreadId = 0; //TEMP FOR DEBUG.

    InitThreadArgs *initThreadArgs = new InitThreadArgs{
        .arguments = arguments,
        .startFunction = startFunction,
        .threadId = &threads[name].posixThreadId,
    };

    pthread_t thread;
    const bool threadWasCreated = (0 == (res = pthread_create(&thread, &attr, initThread, initThreadArgs)));
    if (threadWasCreated) {
        error = ErrorType::Success;
    }
    else {
        deleteThread(name);
        error = fromPlatformError(res);
    }

    _status.threadCount = threads.size();
    pthread_attr_destroy(&attr);

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

    ret = pthread_join(threads[name].posixThreadId, nullptr);
    return fromPlatformError(ret);
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
        return fromPlatformError(errno);
    }
    else {
        semaphores[name] = semaphore;
        return ErrorType::Success;
    }
}

ErrorType OperatingSystem::deleteSemaphore(const std::string &name) {
    std::string internalName = std::string("/").append(name);

    if (0 != sem_unlink(internalName.c_str())) {
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
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::incrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::string internalName = std::string("/").append(name);

    if (0 != sem_post(semaphores[name])) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(const std::string &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::string internalName = std::string("/").append(name);

    if (0 != sem_trywait(semaphores[name])) {
        return fromPlatformError(errno);
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

ErrorType OperatingSystem::idlePercentage(Percent &idlePercent) {
    ErrorType error = ErrorType::Failure;
    std::string idleTime(4, 0);
    std::string cpuTimeSeconds(16, 0);
    std::string elapsedTimeSeconds(16, 0);
    Seconds cpuTime, elapsedTime;

    std::string programName(program_invocation_short_name, strlen(program_invocation_short_name));
    std::string commandCpuTime("ps -p $(pgrep -if ");
    commandCpuTime.append(programName);
    commandCpuTime.append(" | head -1) -o cputimes");
    std::string commandElapsedTime("ps -p $(pgrep -if ");
    commandElapsedTime.append(programName);
    commandElapsedTime.append("| head -1) -o etimes");
    
    FILE* pipe = popen(commandCpuTime.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(cpuTimeSeconds.data(), sizeof(uint8_t), cpuTimeSeconds.capacity(), pipe);
        if (feof(pipe) || bytesRead == cpuTimeSeconds.capacity()) {
            error = ErrorType::Success;
            cpuTimeSeconds.resize(bytesRead);
            while (cpuTimeSeconds.back() == '\n') {
                cpuTimeSeconds.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    cpuTime = strtoul(cpuTimeSeconds.c_str(), nullptr, 10);

    pipe = popen(commandElapsedTime.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(elapsedTimeSeconds.data(), sizeof(uint8_t), elapsedTimeSeconds.capacity(), pipe);
        if (feof(pipe) || bytesRead == elapsedTimeSeconds.capacity()) {
            error = ErrorType::Success;
            elapsedTimeSeconds.resize(bytesRead);
            while (elapsedTimeSeconds.back() == '\n') {
                elapsedTimeSeconds.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    elapsedTime = strtoul(elapsedTimeSeconds.c_str(), nullptr, 10);

    //TODO: This is more like cpu time rather than idle time.
    if (0 != elapsedTime) {
        idlePercent = 100.0f - (((float)cpuTime / (float)elapsedTime) * 100.0f);
    }
    else {
        idlePercent = 100.0f;
    }

    return error;
}

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::string &memoryRegionName) {
    ErrorType error = ErrorType::Failure;

    //Will return the size of RAM in bytes.
    std::string commandFinal("free -b | egrep Mem | tr -s \" \" | cut -d \" \" -f2");
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

    return error;
}

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::string &memoryRegionName) {
    ErrorType error = ErrorType::Failure;

    //Will return the size of available RAM in bytes.
    std::string commandFinal("free -b | egrep Mem | tr -s \" \" | cut -d \" \" -f4");
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

    return error;
}