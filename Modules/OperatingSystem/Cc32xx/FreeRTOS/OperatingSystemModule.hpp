#ifndef __OPERATING_SYSTEM_MODULE_HPP__
#define __OPERATING_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "OperatingSystemAbstraction.hpp"
//Common
#include "Global.hpp"
//C++
#include <map>
//FreeRTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

class OperatingSystem final: public OperatingSystemAbstraction, public Global<OperatingSystem> {

    public:
    OperatingSystem() : OperatingSystemAbstraction(), Global<OperatingSystem>() {
        static_assert(1 == configUSE_TIMERS);
        memoryRegions(_status.memoryRegion);
        savedInterruptContexts.reserve(2);
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

    void callTimerCallback(TimerHandle_t timer);

    ErrorType startSimpleLinkTask();

    size_t toCc32xxPriority(OperatingSystemTypes::Priority priority) {
        switch (priority) {
            case OperatingSystemTypes::Priority::Highest:
                return configMAX_PRIORITIES - 1;
            case OperatingSystemTypes::Priority::High:
                return configMAX_PRIORITIES * 0.8f;
            case OperatingSystemTypes::Priority::Normal:
                return configMAX_PRIORITIES * 0.6f;
            case OperatingSystemTypes::Priority::Low:
                return configMAX_PRIORITIES * 0.4f;
            case OperatingSystemTypes::Priority::Lowest:
                return configMAX_PRIORITIES * 0.2f;
            default:
                assert(false);
        }
    }

    private:
    struct Thread {
        TaskHandle_t cc32xxThreadId;
        std::array<char, OperatingSystemTypes::MaxThreadNameLength> name;
        Id threadId;
    };

    struct Timer {
        std::function<void(void)> callback;
        Id id;
        bool autoReload;
    };

    Id nextTimerId = 0;

    std::map<std::array<char, OperatingSystemTypes::MaxThreadNameLength>, Thread> threads;
    std::map<std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength>, SemaphoreHandle_t> semaphores;
    std::map<std::array<char, OperatingSystemTypes::MaxQueueNameLength>, QueueHandle_t> queues;
    std::map<TimerHandle_t, Timer> timers;
    std::vector<UBaseType_t> savedInterruptContexts;
};

#endif // __OPERATING_SYSTEM_HPP__
