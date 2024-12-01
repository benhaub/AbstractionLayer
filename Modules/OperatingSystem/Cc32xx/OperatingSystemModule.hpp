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

class OperatingSystem : public Global<OperatingSystem>, public OperatingSystemAbstraction {

    public:
    ErrorType delay(Milliseconds delay) override;
    ErrorType createThread(OperatingSystemConfig::Priority priority, std::string name, void * arguments, Bytes stackSize, void *(*startFunction)(void *), Id &number) override;
    ErrorType deleteThread(std::string name) override;
    ErrorType joinThread(std::string name) override;
    ErrorType threadId(std::string name, Id &thread) override;
    ErrorType isDeleted(std::string &name) override;
    ErrorType createSemaphore(Count max, Count initial, std::string name) override;
    ErrorType deleteSemaphore(std::string name) override;
    ErrorType waitSemaphore(std::string &name, Milliseconds timeout) override;
    ErrorType incrementSemaphore(std::string &name) override;
    ErrorType decrementSemaphore(std::string name) override;
    ErrorType createTimer(Id &timer, Milliseconds period, bool autoReload, std::function<void(void)> callback) override;
    ErrorType startTimer(Id timer, Milliseconds timeout) override;
    ErrorType stopTimer(Id timer, Milliseconds timeout) override;
    ErrorType getSystemTime(UnixTime &currentSystemUnixTime) override;
    ErrorType getSystemTick(Ticks &currentSystemTicks) override;
    ErrorType ticksToMilliseconds(Ticks ticks, Milliseconds &timeInMilliseconds) override;
    ErrorType getSoftwareVersion(std::string &softwareVersion) override;
    ErrorType getResetReason(OperatingSystemConfig::ResetReason &resetReason) override;
    ErrorType reset() override;
    ErrorType setTimeOfDay(UnixTime utc, Seconds timeZoneDifferenceUtc) override;
    ErrorType idlePercentage(Percent &idlePercent) override;

    size_t toCc32xxPriority(OperatingSystemConfig::Priority priority) {
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

    private:
    struct Thread {
        pthread_t cc32xxThreadId;
        std::string name;
        Id threadId;
    };

    std::map<std::string, Thread> threads;
    std::map<std::string, sem_t *> semaphores;
};

#endif // __OPERATING_SYSTEM_HPP__
