//Modules
#include "OperatingSystemModule.hpp"
#include "MemoryPool.hpp"
//C++
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
#include <signal.h> //For timers

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(union sigval val);

#ifdef __cplusplus
}
#endif

//Declared global to keep the memory pool header out of the hpp file otherwise it creates some unwanted dependancies
//(the Event library would need to link with memory pool).
namespace {
    MemoryPool<timer_t, sizeof(timer_t)*8> timerIdPool;
}

ErrorType OperatingSystem::delay(const Milliseconds delay) {
    usleep(delay*1000);
    return ErrorType::Success;
}

ErrorType OperatingSystem::delay(const Microseconds delay) {
    usleep(delay);
    return ErrorType::Success;
}

ErrorType OperatingSystem::startScheduler() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::createThread(const OperatingSystemTypes::Priority priority, const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) {
    pthread_attr_t attr;
    sched_param param;
    int res;
    static Id nextThreadId = 1;

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
        pthread_t *const threadId = nullptr;
    };
    auto initThread = [](void *arguments) -> void * {
        InitThreadArgs *initThreadArgs = static_cast<InitThreadArgs *>(arguments);
        *(initThreadArgs->threadId) = pthread_self();
        (initThreadArgs->startFunction)(initThreadArgs->arguments);
        delete initThreadArgs;
        return nullptr;
    };

    ErrorType error = ErrorType::LimitReached;

    if (threads.size() < _MaxThreads) {
        Thread newThread = {
            .name = name,
            .threadId = nextThreadId++,
            .isBlocked = false,
            .blockCount = 0
        };
        pthread_mutex_init(&(newThread.mutex), nullptr);
        pthread_cond_init(&(newThread.conditionVariable), nullptr);

        threads[name] = newThread;
        number = newThread.threadId;

        InitThreadArgs *initThreadArgs = new InitThreadArgs {
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
    }

    return error;
}

//I want to use pthreads since I like the portability of them, however, ESP does not implement pthread_kill.
//The work around is to set the thread in the deatched state and then have the main loops of each thread regularly check their status
//to see if they have been terminated by the operating system, which will set isTerminated when the thread is detached.
ErrorType OperatingSystem::deleteThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    ErrorType error = ErrorType::NoData;

    if (threads.contains(name)) {
        // Remove thread stack from memory regions
        _status.memoryRegion.erase(
            std::remove_if(_status.memoryRegion.begin(), _status.memoryRegion.end(),
                [&name](const OperatingSystemTypes::MemoryRegionInfo &region) {
                    return 0 == strncmp(region.name.data(), name.data(), OperatingSystemTypes::MaxMemoryRegionNameLength);
                }),
            _status.memoryRegion.end());
        
        threads.erase(name);
        error = ErrorType::Success;
    }

    return error;
}

ErrorType OperatingSystem::joinThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
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

ErrorType OperatingSystem::threadId(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, Id &thread) {
    if (threads.contains(name)) {
        thread = threads[name].threadId;
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::isDeleted(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) {
    if (threads.contains(name)) {
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::createSemaphore(const Count max, const Count initial, const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    //The internal name is the name with a leading / to make it a valid semaphore name on POSIX systems.
    //For all other purposes inside this operating system abstraction, the name should be used directly.
    std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> internalName = {'/'};
    strncat(internalName.data(), name.data(), OperatingSystemTypes::MaxSemaphoreNameLength);

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

ErrorType OperatingSystem::deleteSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> internalName = {'/'};
    strncat(internalName.data(), name.data(), OperatingSystemTypes::MaxSemaphoreNameLength);

    if (0 != sem_unlink(internalName.data())) {
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

ErrorType OperatingSystem::incrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> internalName = {'/'};
    strncat(internalName.data(), name.data(), OperatingSystemTypes::MaxSemaphoreNameLength);

    if (0 != sem_post(semaphores[name])) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::decrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) {
    if (!semaphores.contains(name)) {
        return ErrorType::NoData;
    }

    if (0 != sem_trywait(semaphores[name])) {
        return fromPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType OperatingSystem::createTimer(Id &timer, const Milliseconds period, const bool autoReload, std::function<void(void)> callback) {
    struct sigevent signalEvent;
    timer_t *posixTimerId = nullptr;
    //The value of posixTimerId is local to this frame. Need a mempool to make sure it sticks around.
    assert(ErrorType::NoMemory != timerIdPool.allocate(posixTimerId));
    Timer newTimer = {
        .callback = callback,
        .id = _nextTimerId++,
        .posixTimerId = posixTimerId,
        .autoReload = autoReload,
        .period = period
    };

    signalEvent.sigev_notify = SIGEV_THREAD;
    signalEvent.sigev_notify_function = TimerCallback;
    signalEvent.sigev_value.sival_ptr = posixTimerId;
    //Will fail silently (timer doesn't seem to run at all) if this is not set.
    signalEvent.sigev_notify_attributes = nullptr;

    if (0 == timer_create(CLOCK_REALTIME, &signalEvent, posixTimerId)) {
        timers[*posixTimerId] = newTimer;
        timer = newTimer.id;
        return ErrorType::Success;
    }

    return fromPlatformError(errno);
}

ErrorType OperatingSystem::deleteTimer(const Id timer) {
   auto itr = timers.begin();

    while (itr != timers.end()) {
        if (timer == itr->second.id) {
            const timer_t posixTimerId = itr->first;
            timer_delete(posixTimerId);
            if (ErrorType::InvalidParameter == timerIdPool.deallocate(itr->second.posixTimerId)) {
                delete itr->second.posixTimerId;
            }
            timers.erase(itr);
            return ErrorType::Success;
        }
    }

    return ErrorType::NoData;
}

ErrorType OperatingSystem::startTimer(const Id timer, const Milliseconds timeout) {
    struct itimerspec timerSpec;
    timerSpec.it_value.tv_sec = 0;
    ErrorType error = ErrorType::NoData;

    for (const auto &nextTimer : timers) {
        if (nextTimer.second.id == timer) {
            const Timer &timer = nextTimer.second;
            if (timer.autoReload) {
                //Arm the timer by setting any subfield of it_value to a non-zero value
                timerSpec.it_value.tv_sec = 1;
                timerSpec.it_value.tv_nsec = 0;
                //Timer is autoreload, so the period goes into the interval field.
                timerSpec.it_interval.tv_sec = timer.period / 1000;
                timerSpec.it_interval.tv_nsec = 0;
            }
            else {
                timerSpec.it_value.tv_sec = timer.period / 1000;
                timerSpec.it_value.tv_nsec = 0;
                //Both subfeilds are zero, so when the timer expires just once according to it_value.
                timerSpec.it_interval.tv_sec = 0;
                timerSpec.it_interval.tv_nsec = 0;
            }
        }

        //Must not be greater than this according to the docs. Meaning you can't put more than a second into the nsec member.
        assert(timerSpec.it_value.tv_nsec <= 999999999);

        //You probably didn't mean to set these to zero. On Linux the posix timer granularity is a second at minimum so if you set timeout
        //to less than a second the timers will expire right away or never be armed.
        assert(timerSpec.it_value.tv_sec > 0 || timerSpec.it_value.tv_nsec > 0);

        //The flag is zero so the timer times relatively instead expiring when the system clock reaches the value specified by it_value.
        //Last argument is nullptr since we don't want the output parameter of old_time which is the itimerspec that new time replaced.
        if (0 == timer_settime(nextTimer.first, 0, &timerSpec, nullptr)) {
            return ErrorType::Success;
        }
        else {
            perror("timer_settime");
            error = fromPlatformError(errno);
        }
    }

    return error;
}

ErrorType OperatingSystem::stopTimer(const Id timer, const Milliseconds timeout) {
    auto itr = timers.begin();
    while(itr != timers.end()) {
        if (itr->second.id == timer) {
            struct itimerspec timerSpec;
            timerSpec.it_value.tv_sec = timerSpec.it_value.tv_nsec = 0;
            constexpr int timerIsAbsolute = 0;
            constexpr itimerspec *doNotReturnPreviousTime = nullptr;
            if (0 == timer_settime(itr->first, timerIsAbsolute, &timerSpec, doNotReturnPreviousTime)) {
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

ErrorType OperatingSystem::getSystemTick(Ticks &currentSystemTicks) {
    struct tms timeSample;
    currentSystemTicks = static_cast<Ticks>(times(&timeSample));
    return ErrorType::Success;
}

ErrorType OperatingSystem::ticksToMilliseconds(const Ticks ticks, Milliseconds &timeInMilliseconds) {
    timeInMilliseconds = static_cast<Milliseconds>(ticks * (sysconf(_SC_CLK_TCK) / 1000));
    return ErrorType::Success;
}

ErrorType OperatingSystem::millisecondsToTicks(const Milliseconds milli, Ticks &ticks) {
    ticks = static_cast<Ticks>((milli / 1000) * sysconf(_SC_CLK_TCK));
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

ErrorType OperatingSystem::getResetReason(OperatingSystemTypes::ResetReason &resetReason) {
    //There isn't really such thing as a reset reason for most posix systems.
    resetReason = OperatingSystemTypes::ResetReason::Unknown;
    return ErrorType::Success;
}

ErrorType OperatingSystem::reset() {
    return ErrorType::NotAvailable;
}

ErrorType OperatingSystem::setTimeOfDay(const UnixTime utc, const int16_t timeZoneDifferenceUtc) {
    //On systems that use Posix, you shouldn't attempt to set the time of day, and the time that can be obtained
    //using the posix API will already be the correct time that you need as soon as you start your application.
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

ErrorType OperatingSystem::memoryRegionUsage(OperatingSystemTypes::MemoryRegionInfo &region) {
    ErrorType error = ErrorType::Failure;

    // Get total RAM size in bytes
    constexpr std::array<char, 49> totalCommand = {"free -b | egrep Mem | tr -s \" \" | cut -d \" \" -f2"};
    std::array<char, 16> totalRamStr;
    
    FILE* totalPipe = popen(totalCommand.data(), "r");
    if (nullptr != totalPipe) {

        size_t bytesRead = fread(totalRamStr.data(), sizeof(uint8_t), totalRamStr.size(), totalPipe);

        if (feof(totalPipe) || bytesRead == totalRamStr.size()) {

            for (int i = totalRamStr.max_size()-1; i >= 0; i--) {

                if (totalRamStr[i] == '\n') {
                    totalRamStr[i] = '\0';
                }
            }
            
            const Bytes totalRam = std::strtoul(totalRamStr.data(), nullptr, 10);
            
            // Get available RAM in bytes
            constexpr char availableCommand[] = "free -b | egrep Mem | tr -s \" \" | cut -d \" \" -f7";
            std::array<char, 16> availableRamStr;
            
            FILE* availablePipe = popen(availableCommand, "r");

            if (nullptr != availablePipe) {

                const size_t availableBytesRead = fread(availableRamStr.data(), sizeof(uint8_t), availableRamStr.size(), availablePipe);

                if (feof(availablePipe) || availableBytesRead == availableRamStr.size()) {

                    for (int i = availableRamStr.max_size()-1; i >= 0; i--) {

                        if (availableRamStr[i] == '\n') {
                            availableRamStr[i] = '\0';
                        }
                    }
                    
                    const Bytes availableRam = std::strtoul(availableRamStr.data(), nullptr, 10);
                    region.free = (totalRam > 0) ? (Percent(availableRam) / totalRam) * 100.0f : 0.0f;
                    error = ErrorType::Success;
                }

                pclose(availablePipe);
            }
        }

        pclose(totalPipe);
    }

    return error;
}

ErrorType OperatingSystem::uptime(Seconds &uptime) {
    const auto duration = std::chrono::steady_clock::now() - _startTime;
    uptime = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return ErrorType::Success;
}

ErrorType OperatingSystem::disableAllInterrupts() {
    while (_interruptsDisabled) {
        delay(Milliseconds(1));
    }

    _interruptsDisabled = true;
    return ErrorType::Success;
}

ErrorType OperatingSystem::enableAllInterrupts() {
    while (!_interruptsDisabled) {
        delay(Milliseconds(1));
    }

    _interruptsDisabled = false;
    return ErrorType::Success;
}

ErrorType OperatingSystem::block() {
    Id task;
    ErrorType error = currentThreadId(task);

    for (auto &[name, threadStruct] : threads) {

        if (threadStruct.threadId == task) {
            pthread_mutex_lock(&(threadStruct.mutex));
                
            if (threadStruct.blockCount > -1) {
                threadStruct.blockCount++;
                threadStruct.isBlocked = true;

                //pthread_cond_wait will unlock the mutex and lock it again when it returns.
                //The loop is only to protect against spurious wakeups. It's not common to return before the task has been unblocked.
                while (threadStruct.isBlocked) {
                    assert(ErrorType::Success == fromPlatformError(pthread_cond_wait(&threadStruct.conditionVariable, &(threadStruct.mutex))));
                }
            }
            else {
                error = ErrorType::LimitReached;
                threadStruct.blockCount = 0;
            }

            pthread_mutex_unlock(&(threadStruct.mutex));
            
            error = ErrorType::Success;
            break;
        }
        else {
            error = ErrorType::NoData;
        }
    }

    return error;
}

ErrorType OperatingSystem::unblock(const Id task) {
    ErrorType error = ErrorType::NoData;

    for (auto &[name, threadStruct] : threads) {

        if (threadStruct.threadId == task) {
            pthread_mutex_lock(&(threadStruct.mutex));
            threadStruct.blockCount--;

            if (threadStruct.isBlocked) {
                threadStruct.isBlocked = false;
                assert(ErrorType::Success == fromPlatformError(pthread_cond_signal(&(threadStruct.conditionVariable))));
            }

            pthread_mutex_unlock(&(threadStruct.mutex));
            error = ErrorType::Success;
            break;
        }
        else {
            error = ErrorType::NoData;
        }
    }

    return error;
}

void OperatingSystem::callTimerCallback(timer_t *const posixTimerId) {
    assert(nullptr != posixTimerId);

    if (timers.contains(*posixTimerId)) {
        timers[*posixTimerId].callback();

        const bool timerIsOneShot = !timers[*posixTimerId].autoReload;
        if (timerIsOneShot) {
            deleteTimer(timers[*posixTimerId].id);
        }
    }

    return;
}

#ifdef __cplusplus
extern "C" {
#endif

void TimerCallback(union sigval val) {
    OperatingSystem::Instance().callTimerCallback((timer_t *)val.sival_ptr);
}

#ifdef __cplusplus
}
#endif
