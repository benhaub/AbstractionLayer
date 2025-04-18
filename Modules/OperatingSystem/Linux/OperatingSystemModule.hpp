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

class OperatingSystem : public OperatingSystemAbstraction, public Global<OperatingSystem> {

    public:
    OperatingSystem() : OperatingSystemAbstraction(), Global<OperatingSystem>() {
        memoryRegions(_status.memoryRegion);
    }
    ~OperatingSystem() = default;

    ErrorType delay(const Milliseconds delay) override;
    ErrorType delay(const Microseconds delay) override;
    ErrorType startScheduler() override;
    ErrorType createThread(const OperatingSystemConfig::Priority priority, const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) override;
    ErrorType deleteThread(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name) override;
    ErrorType joinThread(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name) override;
    ErrorType threadId(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name, Id &thread) override;
    ErrorType currentThreadId(Id &thread) const override;
    ErrorType isDeleted(const std::array<char, OperatingSystemConfig::MaxThreadNameLength> &name) override;
    ErrorType createSemaphore(const Count max, const Count initial, const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) override;
    ErrorType deleteSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) override;
    ErrorType waitSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name, const Milliseconds timeout) override;
    ErrorType incrementSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) override;
    ErrorType decrementSemaphore(const std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength> &name) override;
    ErrorType createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) override;
    ErrorType deleteTimer(const Id timer) override;
    ErrorType startTimer(Id timer, Milliseconds timeout) override;
    ErrorType stopTimer(Id timer, Milliseconds timeout) override;
    ErrorType createQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, const Bytes size, const Count length) override;
    ErrorType sendToQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) override;
    ErrorType receiveFromQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) override;
    ErrorType peekFromQueue(const std::array<char, OperatingSystemConfig::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) override;
    ErrorType getSystemTime(UnixTime &currentSystemUnixTime) override;
    ErrorType getSystemTick(Ticks &currentSystemTicks) override;
    ErrorType ticksToMilliseconds(const Ticks ticks, Milliseconds &timeInMilliseconds) override;
    ErrorType millisecondsToTicks(const Milliseconds milli, Ticks &ticks) override;
    ErrorType getSoftwareVersion(std::string &softwareVersion) override;
    ErrorType getResetReason(OperatingSystemConfig::ResetReason &resetReason) override;
    ErrorType reset() override;
    ErrorType setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) override;
    ErrorType idlePercentage(Percent &idlePercent) override;
    ErrorType maxHeapSize(Bytes &size, const std::array<char, OperatingSystemConfig::MaxMemoryRegionNameLength> &memoryRegionName) override;
    ErrorType availableHeapSize(Bytes &size, const std::array<char, OperatingSystemConfig::MaxMemoryRegionNameLength> &memoryRegionName) override;
    ErrorType memoryRegions(std::vector<OperatingSystemConfig::MemoryRegionInfo> &memoryRegions) override {
        memoryRegions.clear();
        return ErrorType::Success;
    }
    ErrorType uptime(Seconds &uptime) override;

    int toPosixPriority(OperatingSystemConfig::Priority priority) {
        assert(sched_get_priority_max(SCHED_FIFO) / 2 > 4);

        switch (priority) {
            case OperatingSystemConfig::Priority::Highest:
                return sched_get_priority_max(SCHED_FIFO) / 2;
                break;
            case OperatingSystemConfig::Priority::High:
                return (sched_get_priority_max(SCHED_FIFO) / 2) - 1;
                break;
            case OperatingSystemConfig::Priority::Normal:
                return (sched_get_priority_max(SCHED_FIFO) / 2) - 2;
                break;
            case OperatingSystemConfig::Priority::Low:
                return (sched_get_priority_max(SCHED_FIFO) / 2) - 3;
                break;
            case OperatingSystemConfig::Priority::Lowest:
                return (sched_get_priority_min(SCHED_FIFO) / 2) - 4;
                break;
            default:
                assert(false);
        }
    }

    void callTimerCallback(timer_t *const posixTimerId);

    private:
    struct Thread {
        pthread_t posixThreadId;
        std::array<char, OperatingSystemConfig::MaxThreadNameLength> name;
        Id threadId;
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

    std::map<std::array<char, OperatingSystemConfig::MaxThreadNameLength>, Thread> threads;
    std::map<std::array<char, OperatingSystemConfig::MaxSemaphoreNameLength>, sem_t *> semaphores;
};

#endif // __OPERATING_SYSTEM_HPP__
