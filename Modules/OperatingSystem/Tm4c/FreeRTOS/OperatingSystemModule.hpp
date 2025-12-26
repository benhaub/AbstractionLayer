#ifndef __OPERATING_SYSTEM_MODULE_HPP__
#define __OPERATING_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "OperatingSystemAbstraction.hpp"
//Common
#include "Global.hpp"
//C++
#include <map>
#include <atomic>
//FreeRTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

class OperatingSystem final : public OperatingSystemAbstraction, public Global<OperatingSystem> {

    public:
    OperatingSystem() : OperatingSystemAbstraction(), Global<OperatingSystem>() {
        savedInterruptContexts.reserve(2);

        // Add heap memory region
        constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> heap = {"Heap"};
        _status.memoryRegion.emplace_back(heap);
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
    ErrorType memoryRegionUsage(OperatingSystemTypes::MemoryRegionInfo &region) override;
    ErrorType uptime(Seconds &uptime) override;
    ErrorType disableAllInterrupts() override;
    ErrorType enableAllInterrupts() override;
    ErrorType block() override;
    ErrorType unblock(const Id task) override;
    ErrorType getSystemMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;

    void callTimerCallback(TimerHandle_t timer);

    size_t toTm4c123Priority(OperatingSystemTypes::Priority priority) {
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
        TaskHandle_t tm4c123ThreadId;
        std::array<char, OperatingSystemTypes::MaxThreadNameLength> name;
        Id threadId;
        Bytes maxStackSize;
        OperatingSystemTypes::ThreadStatus status;
    };

    struct Timer {
        std::function<void(void)> callback;
        Id id;
        bool autoReload;
    };

    Id nextTimerId = 0;

    std::array<Thread, APP_MAX_NUMBER_OF_THREADS> threads;
    std::map<std::array<char, OperatingSystemTypes::MaxQueueNameLength>, QueueHandle_t> queues;
    std::map<std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength>, SemaphoreHandle_t> semaphores;
    std::map<TimerHandle_t, Timer> timers;
    std::vector<UBaseType_t> savedInterruptContexts;

    int toThreadIndex(const Id threadId) {
        return (threadId - 1);
    }
};

#endif // __OPERATING_SYSTEM_HPP__
