#ifndef __OPERATING_SYSTEM_MODULE_HPP__
#define __OPERATING_SYSTEM_MODULE_HPP__

//AbstractionLayer
#include "OperatingSystemAbstraction.hpp"
//Common
#include "Global.hpp"
//FreeRtos
//ESP operating system is a significantly modified version of FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FreeRTOSConfig.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
//ESP
#include "esp_system.h"
//C++
#include <map>

class OperatingSystem final : public OperatingSystemAbstraction, public Global<OperatingSystem> {

    public:
    OperatingSystem() : OperatingSystemAbstraction(), Global<OperatingSystem>() {
        static_assert(1 == configUSE_TIMERS);
        static_assert(1 == configUSE_IDLE_HOOK);
        memoryRegions(_status.memoryRegion);
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
    ErrorType setTimeOfDay(const UnixTime utc, const Seconds timeZoneDifferenceUtc) override;
    ErrorType idlePercentage(Percent &idlePercent) override;
    ErrorType maxHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) override;
    ErrorType availableHeapSize(Bytes &size, const std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> &memoryRegionName) override;
    ErrorType memoryRegions(std::vector<OperatingSystemTypes::MemoryRegionInfo> &memoryRegions) override {
        constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> dram = {"DRAM"};
        memoryRegions.emplace_back(dram, 0);
#ifdef CONFIG_SPIRAM
        constexpr std::array<char, OperatingSystemTypes::MaxMemoryRegionNameLength> spiram = {"SPIRAM"};
        memoryRegions.emplace_back(spiram, 0);
#endif
        return ErrorType::Success;
    }
    ErrorType uptime(Seconds &uptime) override;
    ErrorType disableAllInterrupts() override;
    ErrorType enableAllInterrupts() override;

    void callTimerCallback(TimerHandle_t timer);

    private:
    struct Thread {
        TaskHandle_t espThreadId;
        std::array<char, OperatingSystemTypes::MaxThreadNameLength> name;
        Id threadId;
    };

    struct Timer {
        std::function<void(void)> callback;
        Id id;
        bool autoReload;
    };

    std::map<std::array<char, OperatingSystemTypes::MaxThreadNameLength>, Thread> threads;
    std::map<std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength>, SemaphoreHandle_t> semaphores;
    std::map<TimerHandle_t, Timer> timers;
    Id nextTimerId = 0;
    portMUX_TYPE _interruptSpinlock = portMUX_INITIALIZER_UNLOCKED;

    size_t toEspPriority(OperatingSystemTypes::Priority priority) {
        switch (priority) {
            case OperatingSystemTypes::Priority::Highest:
                return configMAX_PRIORITIES-1;
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

    OperatingSystemTypes::ResetReason toPlatformResetReason(uint8_t resetReason, ErrorType &error) {
        error = ErrorType::Success;

        switch (resetReason) {
            case ESP_RST_UNKNOWN:
                return OperatingSystemTypes::ResetReason::Unknown;
            case ESP_RST_POWERON:
                return OperatingSystemTypes::ResetReason::PowerOn;
            case ESP_RST_EXT:
                return OperatingSystemTypes::ResetReason::ExternalPin;
            case ESP_RST_SW:
                return OperatingSystemTypes::ResetReason::Software;
            case ESP_RST_PANIC:
                return OperatingSystemTypes::ResetReason::Exception;
            case ESP_RST_INT_WDT:
            case ESP_RST_TASK_WDT:
            case ESP_RST_WDT:
                return OperatingSystemTypes::ResetReason::Watchdog;
            case ESP_RST_DEEPSLEEP:
                return OperatingSystemTypes::ResetReason::DeepSleep;
            case ESP_RST_BROWNOUT:
                return OperatingSystemTypes::ResetReason::BrownOut;
            case ESP_RST_SDIO:
            default:
                error = ErrorType::Failure;
                return OperatingSystemTypes::ResetReason::Unknown;
        }
    }
};

#endif // __OPERATING_SYSTEM_MODULE_HPP__
