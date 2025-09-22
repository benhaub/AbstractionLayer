#ifndef __OPERATING_SYSTEM_MODULE_HPP__
#define __OPERATING_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "OperatingSystemAbstraction.hpp"
#include "Global.hpp"
//Posix
#include <sched.h>
#include <semaphore.h>
//C++
#include <cassert>
#include <ctime>
#include <map>
#include <chrono>

class OperatingSystem final : public OperatingSystemAbstraction, public Global<OperatingSystem> {

    public:
    OperatingSystem() : OperatingSystemAbstraction(), Global<OperatingSystem>() {
        memoryRegions(_status.memoryRegion);
        _startTime = std::chrono::steady_clock::now();
    }

    ErrorType delay(const Milliseconds delay) override;
    ErrorType delay(const Microseconds delay) override;
    ErrorType startScheduler() override;
    ErrorType createThread(const OperatingSystemTypes::Priority priority, const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) override;
    ErrorType deleteThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) override;
    ErrorType joinThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) override;
    ErrorType threadId(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, Id &thread) override;
    ErrorType currentThreadId(Id &thread) const override;
    ErrorType isDeleted(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) override;
    ErrorType createSemaphore(const Count max, const Count initial, const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) override;
    ErrorType deleteSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) override;
    ErrorType waitSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name, const Milliseconds timeout) override;
    ErrorType incrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) override;
    ErrorType decrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) override;
    ErrorType createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) override;
    ErrorType deleteTimer(const Id timer) override;
    ErrorType startTimer(Id timer, Milliseconds timeout) override;
    ErrorType stopTimer(Id timer, Milliseconds timeout) override;
    ErrorType createQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const Bytes size, const Count length) override;
    ErrorType sendToQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) override;
    ErrorType receiveFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) override;
    ErrorType peekFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) override;
    ErrorType getSystemTime(UnixTime &currentSystemUnixTime) override;
    ErrorType getSystemTick(Ticks &currentSystemTicks) override;
    ErrorType ticksToMilliseconds(const Ticks ticks, Milliseconds &timeInMilliseconds) override;
    ErrorType millisecondsToTicks(const Milliseconds milli, Ticks &ticks) override;
    ErrorType getSoftwareVersion(std::string &softwareVersion) override;
    ErrorType getResetReason(OperatingSystemTypes::ResetReason &resetReason) override;
    ErrorType reset() override;
    ErrorType setTimeOfDay(const UnixTime utc, const int16_t timeZoneDifferenceUtc) override;
    ErrorType idlePercentage(Percent &idlePercent) override;
    ErrorType maxHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) override;
    ErrorType availableHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) override;
    ErrorType memoryRegions(std::vector<OperatingSystemTypes::MemoryRegionInfo> &memoryRegions) override {
        memoryRegions.clear();
        return ErrorType::Success;
    }
    ErrorType uptime(Seconds &uptime) override;
    ErrorType disableAllInterrupts() override;
    ErrorType enableAllInterrupts() override;
    ErrorType block() override;
    ErrorType unblock(const Id task) override;

    int toPosixPriority(OperatingSystemTypes::Priority priority) {
        switch (priority) {
            case OperatingSystemTypes::Priority::Highest:
                return sched_get_priority_max(SCHED_FIFO);
                break;
            case OperatingSystemTypes::Priority::High:
                return (sched_get_priority_max(SCHED_FIFO) * 0.8f);
                break;
            case OperatingSystemTypes::Priority::Normal:
                return (sched_get_priority_max(SCHED_FIFO) * 0.6f);
                break;
            case OperatingSystemTypes::Priority::Low:
                return (sched_get_priority_max(SCHED_FIFO) * 0.4f);
                break;
            case OperatingSystemTypes::Priority::Lowest:
                return (sched_get_priority_max(SCHED_FIFO) * 0.2f);
                break;
            default:
                assert(false);
        }
    }

    void callTimerCallback(timer_t *const posixTimerId);

    private:
    struct Thread {
        pthread_t posixThreadId;
        std::array<char, OperatingSystemTypes::MaxThreadNameLength> name;
        Id threadId;
        bool isBlocked;
        pthread_mutex_t mutex;
        pthread_cond_t conditionVariable;
    };

    struct Timer {
        std::function<void(void)> callback;
        Id id;
        timer_t *posixTimerId;
        bool autoReload;
        Milliseconds period;
    };

    std::map<timer_t, Timer> timers;
    Id _nextTimerId = 0;

    std::map<std::array<char, OperatingSystemTypes::MaxThreadNameLength>, Thread> threads;
    std::map<std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength>, sem_t *> semaphores;

    bool _interruptsDisabled = false;

    std::chrono::steady_clock::time_point _startTime;
};

#endif // __OPERATING_SYSTEM_HPP__
