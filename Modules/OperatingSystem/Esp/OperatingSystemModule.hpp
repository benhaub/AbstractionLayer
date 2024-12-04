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

class OperatingSystem : public Global<OperatingSystem>, public OperatingSystemAbstraction {

    public:
    OperatingSystem() : Global<OperatingSystem>(), OperatingSystemAbstraction() {}
    ~OperatingSystem() = default;

    ErrorType delay(Milliseconds delay) override;
    ErrorType createThread(OperatingSystemConfig::Priority priority, std::string name, void * arguments, Bytes stackSize, void *(*startFunction)(void *), Id &number) override;
    ErrorType deleteThread(std::string name) override;
    ErrorType joinThread(std::string name) override;
    ErrorType threadId(std::string name, Id &id) override;
    ErrorType isDeleted(std::string &name) override;
    ErrorType createSemaphore(Count max, Count initial, std::string name) override;
    ErrorType deleteSemaphore(std::string name) override;
    ErrorType waitSemaphore(std::string &name, Milliseconds timeout) override;
    ErrorType incrementSemaphore(std::string &name) override;
    ErrorType decrementSemaphore(std::string name) override;
    ErrorType createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) override;
    ErrorType deleteTimer(const Id timer) override;
    ErrorType startTimer(Id timer, Milliseconds timeout) override;
    ErrorType stopTimer(Id timer, Milliseconds timeout) override;
    ErrorType getSystemTime(UnixTime &currentSystemUnixTime) override;
    ErrorType ticksToMilliseconds(Ticks ticks, Milliseconds &timeInMilliseconds) override;
    ErrorType getSystemTick(Ticks &currentSystemTicks) override;
    ErrorType getSoftwareVersion(std::string &softwareVersion) override;
    ErrorType getResetReason(OperatingSystemConfig::ResetReason &resetReason) override;
    ErrorType reset() override;
    ErrorType setTimeOfDay(UnixTime utc, Seconds timeZoneDifferenceUtc) override;
    ErrorType idlePercentage(Percent &idlePercent) override;

    void callTimerCallback(TimerHandle_t timer);

    private:
    struct Thread {
        pthread_t posixThreadId;
        std::string name;
        Id threadId;
    };

    struct Timer {
        std::function<void(void)> callback;
        Id id;
        bool autoReload;
    };

    std::map<std::string, Thread> threads;
    std::map<std::string, SemaphoreHandle_t> semaphores;
    std::map<TimerHandle_t, Timer> timers;
    Id nextTimerId = 0;

    size_t toEspPriority(OperatingSystemConfig::Priority priority) {
        assert(configMAX_PRIORITIES >= 20);

        switch (priority) {
            case OperatingSystemConfig::Priority::Highest:
                return configMAX_PRIORITIES;
            case OperatingSystemConfig::Priority::High:
                return configMAX_PRIORITIES - 5;
            case OperatingSystemConfig::Priority::Normal:
                return configMAX_PRIORITIES - 10;
            case OperatingSystemConfig::Priority::Low:
                return configMAX_PRIORITIES - 15;
            case OperatingSystemConfig::Priority::Lowest:
                return configMAX_PRIORITIES - 20;
            default:
                assert(false);
        }
    }

    OperatingSystemConfig::ResetReason toCbtResetReason(uint8_t resetReason, ErrorType &error) {
        error = ErrorType::Success;

        switch (resetReason) {
            case ESP_RST_UNKNOWN:
                return OperatingSystemConfig::ResetReason::Unknown;
            case ESP_RST_POWERON:
                return OperatingSystemConfig::ResetReason::PowerOn;
            case ESP_RST_EXT:
                return OperatingSystemConfig::ResetReason::ExternalPin;
            case ESP_RST_SW:
                return OperatingSystemConfig::ResetReason::Software;
            case ESP_RST_PANIC:
                return OperatingSystemConfig::ResetReason::Exception;
            case ESP_RST_INT_WDT:
            case ESP_RST_TASK_WDT:
            case ESP_RST_WDT:
                return OperatingSystemConfig::ResetReason::Watchdog;
            case ESP_RST_DEEPSLEEP:
                return OperatingSystemConfig::ResetReason::DeepSleep;
            case ESP_RST_BROWNOUT:
                return OperatingSystemConfig::ResetReason::BrownOut;
            case ESP_RST_SDIO:
                return OperatingSystemConfig::ResetReason::Sdio;
            default:
                error = ErrorType::Failure;
                return OperatingSystemConfig::ResetReason::Unknown;
        }
    }
};

#endif // __CBT_OPERATING_SYSTEM_HPP__
