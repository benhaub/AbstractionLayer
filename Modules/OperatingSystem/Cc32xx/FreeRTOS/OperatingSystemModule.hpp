#ifndef __OPERATING_SYSTEM_MODULE_HPP__
#define __OPERATING_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "OperatingSystemAbstraction.hpp"
//Common
#include "Global.hpp"
//C++
#include <map>
//Posix
#include <semaphore.h>
//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

class OperatingSystem : public OperatingSystemAbstraction, public Global<OperatingSystem> {

    public:
    OperatingSystem() : OperatingSystemAbstraction(), Global<OperatingSystem>() {
        memoryRegions(_status.memoryRegion);
    }
    ~OperatingSystem() = default;

    ErrorType delay(const Milliseconds delay) override;
    ErrorType delay(const Microseconds delay) override;
    ErrorType startScheduler() override;
    ErrorType createThread(const OperatingSystemConfig::Priority priority, const std::string &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) override;
    ErrorType deleteThread(const std::string &name) override;
    ErrorType joinThread(const std::string &name) override;
    ErrorType threadId(const std::string &name, Id &thread) override;
    ErrorType currentThreadId(Id &thread) const override;
    ErrorType isDeleted(const std::string &name) override;
    ErrorType createSemaphore(const Count max, const Count initial, const std::string &name) override;
    ErrorType deleteSemaphore(const std::string &name) override;
    ErrorType waitSemaphore(const std::string &name, const Milliseconds timeout) override;
    ErrorType incrementSemaphore(const std::string &name) override;
    ErrorType decrementSemaphore(const std::string &name) override;
    ErrorType createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) override;
    ErrorType deleteTimer(const Id timer) override;
    ErrorType startTimer(Id timer, Milliseconds timeout) override;
    ErrorType stopTimer(Id timer, Milliseconds timeout) override;
    ErrorType createQueue(const std::string &name, const Bytes size, const Count length) override;
    ErrorType sendToQueue(const std::string &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) override;
    ErrorType receiveFromQueue(const std::string &name, void *buffer, const Milliseconds timeout, const bool fromIsr) override;
    ErrorType peekFromQueue(const std::string &name, void *buffer, const Milliseconds timeout, const bool fromIsr) override;
    ErrorType getSystemTime(UnixTime &currentSystemUnixTime) override;
    ErrorType getSystemTick(Ticks &currentSystemTicks) override;
    ErrorType ticksToMilliseconds(const Ticks ticks, Milliseconds &timeInMilliseconds) override;
    ErrorType millisecondsToTicks(const Milliseconds milli, Ticks &ticks) override;
    ErrorType getSoftwareVersion(std::string &softwareVersion) override;
    ErrorType getResetReason(OperatingSystemConfig::ResetReason &resetReason) override;
    ErrorType reset() override;
    ErrorType setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) override;
    ErrorType idlePercentage(Percent &idlePercent) override;
    ErrorType maxHeapSize(Bytes &size, const std::string &memoryRegionName) override;
    ErrorType availableHeapSize(Bytes &size, const std::string &memoryRegionName) override;
    ErrorType memoryRegions(std::vector<OperatingSystemConfig::MemoryRegionInfo> &memoryRegions) override {
        memoryRegions.clear();
        return ErrorType::Success;
    }
    ErrorType uptime(Seconds &uptime) override;

    void callTimerCallback(TimerHandle_t timer);

    size_t toCc32xxPriority(OperatingSystemConfig::Priority priority) {
        static_assert(configMAX_PRIORITIES >= 5);

        switch (priority) {
            case OperatingSystemConfig::Priority::Highest:
                return configMAX_PRIORITIES - 1;
            case OperatingSystemConfig::Priority::High:
                return configMAX_PRIORITIES - 2;
            case OperatingSystemConfig::Priority::Normal:
                return configMAX_PRIORITIES - 3;
            case OperatingSystemConfig::Priority::Low:
                return configMAX_PRIORITIES - 4;
            case OperatingSystemConfig::Priority::Lowest:
                return configMAX_PRIORITIES - 5;
            default:
                assert(false);
        }
    }

    private:
    struct Thread {
        pthread_t cc32xxThreadId;
        std::string name;
        Id threadId;
    };

    struct Timer {
        std::function<void(void)> callback;
        Id id;
        bool autoReload;
    };

    Id nextTimerId = 0;

    std::map<std::string, Thread> threads;
    std::map<std::string, sem_t> semaphores;
    std::map<std::string, QueueHandle_t> queues;
    std::map<TimerHandle_t, Timer> timers;
};

#endif // __OPERATING_SYSTEM_HPP__
