//Modules
#include "OperatingSystemModule.hpp"
//Posix
#include <sys/times.h> //For timesample and system tick queries
#include <sys/syslimits.h> //For max semaphore name length

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(void *arg);

#ifdef __cplusplus
}
#endif

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    usleep(delay*1000);
    return ErrorType::Success;
}


ErrorType OperatingSystem::delay(Microseconds delay) {
    usleep(delay);
    return ErrorType::Success;
}

ErrorType OperatingSystem::startScheduler() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::createThread(const OperatingSystemConfig::Priority priority, const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
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

        if (threads.size() < _MaxThreads) {
            threads[name] = newThread;
            return ErrorType::Success;
        }
        else {
            deleteThread(name);
            return ErrorType::LimitReached;
        }
    }
    else {
        return fromPlatformError(res);
    }
}

ErrorType OperatingSystem::deleteThread(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        threads.erase(name);
    }

    return error;
}

ErrorType OperatingSystem::joinThread(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name) {
    Id thread;
    if (ErrorType::NoData == threadId(name, thread)) {
        return ErrorType::NoData;
    }

    return fromPlatformError(pthread_join(threads[name].posixThreadId, nullptr));
}

ErrorType OperatingSystem::threadId(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name, Id &thread) {
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

ErrorType OperatingSystem::isDeleted(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name) {
    if (threads.contains(name)) {
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::createSemaphore(const Count max, const Count initial, const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) {
    //The internal name is the name with a leading / to make it a valid semaphore name on POSIX systems.
    //For all other purposes inside this operating system abstraction, the name should be used directly.
    std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> internalName = {'/'};
    strncat(internalName.data(), name.data(), OperatingSystemConfig::MaxSemaphoreNameLength - 1);

    if (internalName.size() > NAME_MAX-4) {
        return ErrorType::InvalidParameter;
    }

    //On POSIX systems, a created semaphore has peristence within the kernel until it is removed.
    //So delete old semaphores first and then create the new one since we specify O_EXCL as a flag.
    deleteSemaphore(name); //Using name and not internalName is NOT a bug.
    sem_t *semaphore = sem_open(internalName.data(), O_CREAT | O_EXCL, S_IRWXU, initial);
    if (SEM_FAILED == semaphore) {
        return fromPlatformError(errno);
    }
    else {
        semaphores[name] = semaphore;
        return ErrorType::Success;
    }
}

ErrorType OperatingSystem::deleteSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) {
    std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> internalName = {'/'};
    strncat(internalName.data(), name.data(), OperatingSystemConfig::MaxSemaphoreNameLength - 1);

    if (0 != sem_unlink(internalName.data())) {
        return fromPlatformError(errno);
    }

    semaphores.erase(name);

    return ErrorType::Success;
}

ErrorType OperatingSystem::waitSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name, const Milliseconds timeout) {
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

ErrorType OperatingSystem::incrementSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> internalName = {'/'};
    strncat(internalName.data(), name.data(), OperatingSystemConfig::MaxSemaphoreNameLength - 1);

    if (0 != sem_post(semaphores[name])) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_trywait(semaphores[name])) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::createTimer(Id &timer, const Milliseconds period, const bool autoReload, std::function<void(void)> callback) {
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_source_t dispatchTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    
    if (dispatchTimer == nullptr) {
        return ErrorType::Failure;
    }

    Timer newTimer = {
        .callback = callback,
        .id = nextTimerId++,
        .autoReload = autoReload,
        .period = period,
        .isSuspended = false,
        .isResumed = true //When the timer is activated, it's already resumed.
    };

    timers[dispatchTimer] = newTimer;
    
    dispatch_source_set_event_handler_f(dispatchTimer, TimerCallback);
    dispatch_set_context(dispatchTimer, dispatchTimer);
    dispatch_activate(dispatchTimer);
    
    return ErrorType::Success;
}

ErrorType OperatingSystem::deleteTimer(const Id timer) {
    auto it = std::find_if(timers.begin(), timers.end(),
        [timer](const auto& pair) { return pair.second.id == timer; });
    
    if (it == timers.end()) {
        return ErrorType::NoData;
    }

    //It is not a bug the we have not stopped the timer before deleting.
    //https://developer.apple.com/documentation/dispatch/dispatchobject/suspend()?language=objc
    dispatch_source_cancel(it->first);
    dispatch_release(it->first);
    timers.erase(it);
    
    return ErrorType::Success;
}

ErrorType OperatingSystem::startTimer(const Id timer, const Milliseconds timeout) {
    auto it = std::find_if(timers.begin(), timers.end(),
        [timer](const auto& pair) { return pair.second.id == timer; });
    
    if (it == timers.end()) {
        return ErrorType::NoData;
    }

    dispatch_source_t dispatchTimer = it->first;
    Timer& timerData = it->second;

    const uint64_t interval = timerData.period * NSEC_PER_MSEC;
    const dispatch_time_t startTime = dispatch_time(DISPATCH_TIME_NOW, timeout * NSEC_PER_MSEC);
    const uint64_t leeway = NSEC_PER_MSEC/2;
    dispatch_source_set_timer(dispatchTimer, startTime, timerData.autoReload ? interval : DISPATCH_TIME_FOREVER, leeway);

    const bool timerWasNotPreviouslyStopped = !timerData.isResumed;
    if (timerWasNotPreviouslyStopped) {
        dispatch_resume(dispatchTimer);
        timerData.isResumed = true;
    }

    timerData.isSuspended = false;
    return ErrorType::Success;
}

ErrorType OperatingSystem::stopTimer(const Id timer, const Milliseconds timeout) {
    auto it = std::find_if(timers.begin(), timers.end(),
        [timer](const auto& pair) { return pair.second.id == timer; });
    
    if (it == timers.end()) {
        return ErrorType::NoData;
    }

    if (it->second.isSuspended) {
        return ErrorType::Success;
    }

    dispatch_suspend(it->first);
    it->second.isResumed = false;
    it->second.isSuspended = true;

    return ErrorType::Success;
}

ErrorType OperatingSystem::createQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, const Bytes size, const Count length) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::sendToQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::receiveFromQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
    return ErrorType::NotImplemented;
}

ErrorType OperatingSystem::peekFromQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) {
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
    //There isn't really such thing as a reset reason for most posix systems.
    resetReason = OperatingSystemConfig::ResetReason::Unknown;
    return ErrorType::Success;
}

ErrorType OperatingSystem::reset() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) {
    //On systems that use Posix, you shouldn't attempt to set the time of day, and the time that can be obtained
    //using the posix API will already be the correct time that you need as soon as you start your application.
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

ErrorType OperatingSystem::maxHeapSize(Bytes &size, const std::array<char, OperatingSystemConfig::MaxMemoryRegionNameLength> &memoryRegionName) {
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

ErrorType OperatingSystem::availableHeapSize(Bytes &size, const std::array<char, OperatingSystemConfig::MaxMemoryRegionNameLength> &memoryRegionName) {
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

    Bytes totalRam;
    maxHeapSize(totalRam, memoryRegionName);

    size = totalRam - std::strtoul(ramUsed.c_str(), nullptr, 10);

    return error;
}

ErrorType OperatingSystem::uptime(Seconds &uptime) {
    ErrorType error = ErrorType::Failure;

    std::string programName(getprogname(), strlen(getprogname()));
    std::string commandFinal = "ps -p $(pgrep -if ";
    commandFinal.append(programName);
    commandFinal.append("| head -1) -o etimes | tail -1 | tr -d \" \"");
    std::string elapsedTime(16, 0);
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(elapsedTime.data(), sizeof(uint8_t), elapsedTime.capacity(), pipe);
        if (feof(pipe) || bytesRead == elapsedTime.capacity()) {
            error = ErrorType::Success;
            elapsedTime.resize(bytesRead);
            while (elapsedTime.back() == '\n') {
                elapsedTime.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    uptime = std::strtoul(elapsedTime.data(), nullptr, 10);

    return error;
}

void OperatingSystem::callTimerCallback(const dispatch_source_t macOsTimerId) {
    assert(nullptr != macOsTimerId);

    if (timers.contains(macOsTimerId)) {
        timers[macOsTimerId].callback();

        const bool timerIsOneShot = !timers[macOsTimerId].autoReload;
        if (timerIsOneShot) {
            deleteTimer(timers[macOsTimerId].id);
        }
    }

    return;
}

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(void *val) {
    OperatingSystem::Instance().callTimerCallback((dispatch_source_t)val);
}

#ifdef __cplusplus
}
#endif