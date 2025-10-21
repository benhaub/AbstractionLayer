/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   OperatingSystemAbstraction.hpp
* @details Encapsulates the operating system. Provides info and functions.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __OPERATING_SYSTEM_ABSTRACTION_HPP__
#define __OPERATING_SYSTEM_ABSTRACTION_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
#include "Log.hpp"
#include "NetworkTypes.hpp"
//C++
#include <functional>
#include <string>
#include <algorithm>
#include <cassert>

#ifndef APP_MAX_NUMBER_OF_THREADS
#error APP_MAX_NUMBER_OF_THREADS must be defined so that the list of threads is large enough.
#endif

/**
 * @namespace OperatingSystemTypes
 * @brief Types for the operating system
 */
namespace OperatingSystemTypes {

    constexpr Bytes MaxThreadNameLength = 16;       ///< The maximum length of a thread name
    constexpr Bytes MaxSemaphoreNameLength = 16;    ///< The maximum length of a semaphore name
    constexpr Bytes MaxQueueNameLength = 16;        ///< The maximum length of a queue name
    constexpr Bytes MaxMemoryRegionNameLength = 16; ///< The maximum length of a memory region name
    constexpr Id NullId = 0;                        ///< The null id that will never be used to identify a thread.

    /**
     * @enum Priority
     * @brief The priority of a thread. Determines the preference of the scheduler for giving this thread CPU time.
    */
    enum class Priority : uint8_t {
        Unknown = 0, ///< Unknown priority
        Highest,     ///< Highest priority
        High,        ///< High priority
        Normal,      ///< Normal priority
        Low,         ///< Low priority
        Lowest       ///< Lowest priority
    };

    /**
     * @enum ResetReason
     * @brief The reason the processor was reset.
    */
    enum class ResetReason : uint8_t {
        Unknown,     ///< Reset reason can not be determined
        PowerOn,     ///< Reset due to power-on event
        ExternalPin, ///< Reset by external pin
        Software,    ///< Deliberate software reset
        Exception,   ///< Software reset due to exception/panic
        Watchdog,    ///< Reset (software or hardware) due to interrupt watchdog
        DeepSleep,   ///< Reset after exiting deep sleep mode
        BrownOut,    ///< Brownout reset (software or hardware)
        Update       ///< Reset due to completion of an update
    };

    /**
     * @enum MemoryRegionName
     * @brief The name of a memory region.
     */
    enum class MemoryRegionName : uint8_t {
        Unknown,
        Heap,
        Stack
    };

    /**
     * @enum ThreadStatus
     * @brief The status of a thread.
     */
    enum class ThreadStatus : uint8_t {
        Unknown,    ///< The status of the thread is unknown.
        Blocked,    ///< The thread is blocked.
        Terminated, ///< The thread is terminated.
        Active      ///< The thread is active.
    };

    /**
     * @brief MemoryRegionInfo
     */

    /**
     * @struct MemoryRegionInfo
     * @brief The free memory in each memory region.
     */
    struct MemoryRegionInfo {
        std::array<char, MaxMemoryRegionNameLength> name; ///< The name of the memory region
        Percent free;                                     ///< The free memory in the region

        MemoryRegionInfo(const std::array<char, MaxMemoryRegionNameLength> &name) : name(name), free(0) {}
    };


    /**
     * @struct Status
     * @brief The status of the operating system
     */
    struct Status {
        Count threadCount;                          ///< The number of threads currently running.
        Percent idle;                               ///< The percent of time the system spent in idle mode. Definition varies depending on the underlying operating sytem.
        Seconds upTime;                             ///< The amount of time since the system was last reset.
        std::vector<MemoryRegionInfo> memoryRegion; ///< Free memory on the system.
        UnixTime systemTime;                        ///< The current system time.
    };
}

/**
 * @class OperatingSystemAbstraction
 * @brief An interface for the operating system.
*/
class OperatingSystemAbstraction {

    public:
    /// @brief Default constructor
    OperatingSystemAbstraction() = default;
    /// @brief Default destructor
    virtual ~OperatingSystemAbstraction() = default;

    /// @brief The tag for logging.
    static constexpr char TAG[] = "OperatingSystem";
    /// @brief The maximum value for a counting semaphore.
    static constexpr Count MaxCountingSemaphore = 10;

    /// @brief Print that status of the operating system
    void printStatus() {
        status(true);

        PLT_LOGI(TAG, "<OperatingSystemStatus> <Thread Count:%d, Idle (%%):%.1f, Up Time (s):%d, System UnixTime (UTC):%u> <Stairs, Line, Omit, Omit>",
        status().threadCount, status().idle, status().upTime, status().systemTime);
        for (const auto &memoryRegion : status().memoryRegion) {
            PLT_LOGI(TAG, "<Memory Region:%s> <Free (%%):%.1f> <Line>",
            memoryRegion.name.data(), memoryRegion.free);
        }
    }

    /**
     * @brief delays a thread by placing it in the blocking state.
     * @param[in] delay The amount of time to block for.
     * @returns ErrorType::Success if the thread is successfully blocked.
    */
    virtual ErrorType delay(const Milliseconds delay) = 0;
    /**
     * @brief delays a thread by placing it in the blocking state.
     * @param[in] delay The amount of time to block for.
     * @returns ErrorType::Success if the thread is successfully blocked.
    */
    virtual ErrorType delay(const Microseconds delay) = 0;
    /**
     * @brief Start the scheduler
     * @post Typically never returns. Depends on the implementation.
     * @returns ErrorType::Success if the scheduler was started.
     * @returns ErrorType::Failure if the scheduler was not started.
     */
    virtual ErrorType startScheduler() = 0;
    /**
     * @brief Create a new thread.
     * @param[in] priority The priority of the new thread.
     * @param[in] name The name of the thread.
     * @param[in] arguments The argument to pass to the thread.
     * @param[in] stackSize The stack size of the thread.
     * @param[in] startFunction The function called by the operating system to start the thread.
     * @param[out] number The id of the new thread.
     * @returns ErrorType::Success if the thread is successfully created.
     * @returns ErrorType::NotImplemented if createThread is not implemented.
     * @returns ErrorType::LimitReached if the maximum number of threads is reached.
     * @returns toPlatformError for all other errors produced by the underlying implementation
    */
    virtual ErrorType createThread(const OperatingSystemTypes::Priority priority, const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, void * arguments, const Bytes stackSize, void *(*startFunction)(void *), Id &number) = 0;
    /**
     * @brief Delete a thread
     * @param[in] name The name of the thread to delete.
    */
    virtual ErrorType deleteThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) = 0;
    /**
     * @brief Blocks the calling thread until the thread given by name terminates
     * @param[in] name The name of the thread to wait for.
     * @returns ErrorType::Success if the thread is successfully created.
     * @returns ErrorType::NotImplemented if joinThread is not implemented.
     * @returns ErrorType::NoData if no thread with the name given has been created.
     * @returns toPlatformError() for all other errors produced by the underlying implementation
    */
    virtual ErrorType joinThread(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) = 0;
    /**
     * @brief Gets the Id of the thread given by name.
     * @param[in] name The name of the thread.
     * @param[out] id The id of the thread.
     * @returns ErrorType::Success if the thread Id was found.
     * @returns ErrorType::NotImplemented if threadId is not implemented
     * @returns ErrorType::NoData if no thread with the name given has been created.
     * @returns toPlatformError() for all other errors produced by the underlying implementation
    */
    virtual ErrorType threadId(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name, Id &id) = 0;
    /**
     * @brief Get the id of the currently running thread.
     * @param[out] id The id of the currently running thread.
     * @returns ErrorType::Success if the id was found.
     * @returns ErrorType::NotImplemented if threadId is not implemented.
     * @returns ErrorType::NoData if no thread with the name given has been created.
     * @returns ErrorType::Failure otherwise.
    */
    virtual ErrorType currentThreadId(Id &id) const = 0;
    /**
     * @brief Check if the thread has been deleted by the operating system.
     * @details The operating system keeps a record of the threads it has created. Since the OperatingSystemAbstraction is decoupled from the main application,
     *          it is not able to delete threads directly since it's not possible to tell when it is safe to delete a thread. Instead, it marks the thread as deleted
     *          so that the main application can check if it's been deleted and then delete the thread when it's safe to do so.
     * @param[in] name The name of the thread.
     * @returns ErrorType::Success if the thread has been deleted.
     * @returns ErrorType::NotImplemented if isDeleted is not implemented.
     * @returns ErrorType::Negative if the thread has not been deleted.
    */
    virtual ErrorType isDeleted(const std::array<char, OperatingSystemTypes::MaxThreadNameLength> &name) = 0;
    /**
     * @brief creates a semaphore.
     * @param[in] max The maximum value of the semaphore.
     * @param[in] initial The initial value of the semaphore.
     * @param[in] name The name of the semaphore.
    */
    virtual ErrorType createSemaphore(const Count max, const Count initial, const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) = 0;
    /**
     * @brief deletes a semaphore.
     * @param[in] name The name of the semaphore.
     * @returns ErrorType::Success if the semaphore was deleted.
     * @returns ErrorType::NoData if the semaphore does not exist.
    */
    virtual ErrorType deleteSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) = 0;
    /**
     * @brief waits for a semaphore.
     * @param[in] name The name of the semaphore.
     * @param[in] timeout The amount of time to wait for the semaphore.
     * @returns ErrorType::Timeout if the semaphore couldn't be decremented within the timeout likely because the value is already zero.
     * @returns ErrorType::Success if the was decremented.
     * @returns ErrorType::NoData if the semaphore does not exist.
    */
    virtual ErrorType waitSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name, const Milliseconds timeout) = 0;
    /**
     * @brief increments a semaphore.
     * @param[in] name The name of the semaphore.
     * @returns ErrorType::Success if the semaphore was incremented
     * @returns ErrorType::NoData if the semaphore does not exist.
    */
    virtual ErrorType incrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) = 0;
    /**
     * @brief decrements a semaphore.
     * @param[in] name The name of the semaphore.
     * @returns ErrorType::Success if the semaphore was incremented
     * @returns ErrorType::NoData if the semaphore does not exist.
    */
    virtual ErrorType decrementSemaphore(const std::array<char, OperatingSystemTypes::MaxSemaphoreNameLength> &name) = 0;
    /**
     * @brief Create a timer.
     * @param[out] timer The id of the timer.
     * @param[in] period The period of the timer.
     * @param[in] autoReload Whether the timer should start again automatically after it's timed out.
     * @param[in] callback The function to call when the timer times out.
     * @returns ErrorType::Success if the timer is successfully created.
     * @returns ErrorType::Failure if the timer could not be created.
    */
    virtual ErrorType createTimer(Id &timer, const Milliseconds period, const bool autoReload, std::function<void(void)> callback) = 0;
    /**
     * @brief Delete a timer
     * @param[in] timer The id of the timer to delete
     * @returns ErrorType::Success if the timer could be deleted
     * @returns ErrorType::NoData if the timer id does not exist
     */
    virtual ErrorType deleteTimer(const Id timer) = 0;
    /**
     * @brief start timer.
     * @param[in] timer The id of the timer.
     * @param[in] timeout The time to wait for the timer to start.
     * @returns ErrorType::Success if the timer could be started.
     * @returns ErrorType::Timeout if the timer could not be started within the time specified.
     * @returns ErrorType::NoData if the timer Id was not used to create a prior to this call or it has been deleted.
    */
    virtual ErrorType startTimer(const Id timer, const Milliseconds timeout) = 0;
    /**
     * @brief stop timer.
     * @param[in] timer The id of the timer.
     * @param[in] timeout The time to wait for the timer to stop.
     * @returns ErrorType::Success if the timer could be stopped.
     * @returns ErrorType::Timeout if the timer could not be stopped within the time specified.
    */
    virtual ErrorType stopTimer(const Id timer, const Milliseconds timeout) = 0;
    /**
     * @brief Create a queue to safely send and receive information between threads
     * @param[in] name The name of the queue which you can use to reference the queue
     * @param[in] size The size in bytes of each queue item
     * @param[in] length The number of items that can be in the queue at once.
     * @returns ErrorType::Success if the queue was created
     * @returns ErrorType::NoMemory if the queue can not be created with the specified size and/or length
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType createQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const Bytes size, const Count length) = 0;
    /**
     * @brief Insert data into the queue
     * @param[in] name The name of the queue to send data to.
     * @param[in] data The data to send
     * @param[in] timeout The time to wait for the data to be sent to the queue.
     * @param[in] toFront True if the message should be sent to the front of the queue rather than the back.
     * @param[in] fromIsr Set to true if sending to this queue is done in ISR context.
     * @returns ErrorType::Success if the item was sent to the queue
     * @returns ErrorType::Timeout if the item could not be sent to the queue in time.
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType sendToQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, const void *data, const Milliseconds timeout, const bool toFront, const bool fromIsr) = 0;
    /**
     * @brief Receive data from the queue
     * @param[in] name The name of the queue to receive data from
     * @param[out] buffer The buffer to store the queue item in.
     * @param[in] timeout The time to wait until the item can be retrieved
     * @param[in] fromIsr Set to true if receiving from this queue is done in ISR context.
     * @returns ErrorType::Success if the item was received from the queue
     * @returns ErrorType::Timeout if the item was not received from the queue in time.
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType receiveFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) = 0;
    /**
     * @brief Read from the queue without removing an item from it
     * @param[in] name The name of the queue to peek from
     * @param[in] buffer The buffer to store the queue item in.
     * @param[in] timeout The time to wait until the item can be retrieved.
     * @param[in] fromIsr Set to true if peeking from this queue is done in ISR context
     * @returns ErrorType::Success if the item was peeked from the queue
     * @returns ErrorType::Timeout if the item was not peeked from the queue in time.
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType peekFromQueue(const std::array<char, OperatingSystemTypes::MaxQueueNameLength> &name, void *buffer, const Milliseconds timeout, const bool fromIsr) = 0;
    /**
     * @brief Get the current system time
     * @param[out] currentSystemUnixTime The current unix time of the system.
     * @returns ErrorType::Success if the system unix time was obtained
     * @returns ErrorType::NotImplemented if getting the system time is not implemented
    */
    virtual ErrorType getSystemTime(UnixTime &currentSystemUnixTime) = 0;
    /**
     * @brief Get the current system tick value
     * @param[out] currentSystemTicks The current tick value of the system.
     * @returns ErrorType::Success if the system tick value was obtained
     * @returns ErrorType::NotImplemented if getting the system tick value is not implemented
     * @returns ErrorType::Failure if the system tick value could not be obtained
     */
    virtual ErrorType getSystemTick(Ticks &currentSystemTicks) = 0;
    /**
     * @brief Convert a tick value to a time in milliseconds
     * @param[in] ticks The tick value to convert.
     * @param[out] tickEquivalent The converted tick value.
     * @returns ErrorType::Success if the tick was converted to milliseconds
     * @returns ErrorType::NotImplemented if converting the tick to milliseconds is not implemented
     * @returns ErrorType::Failure if the tick could not be converted to milliseconds
     */
    virtual ErrorType ticksToMilliseconds(const Ticks ticks, Milliseconds &tickEquivalent) = 0;
    /**
     * @brief Convert a millisecond value to a time in ticks
     * @param[in] milli The time to convert
     * @param[out] ticks The converted time in ticks
     * @returns ErrorType::Success if the value was converted
     * @returns ErrorType::NotImplemented if converting milliseconds to ticks is not implemented
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType millisecondsToTicks(const Milliseconds milli, Ticks &ticks) = 0;
    /**
     * @brief Get software version
     * @param[out] softwareVersion The software version in the format a.b.c.d, where a, b, c, and d
     *             are the version numbers in the range (0,10].
     * @returns ErrorType::Success if the software version was obtained
     * @returns ErrorType::NotImplemented if getting the software version is not implemented
    */
    virtual ErrorType getSoftwareVersion(std::string &softwareVersion) = 0;
    /**
     * @brief Get the reset reason from the last time the processor was reset.
     * @param[out] resetReason The reset reason.
     * @returns ErrorType::Success if the reset reason was obtained
     * @returns ErrorType::NotImplemented if getting the reset reason is not implemented
    */
    virtual ErrorType getResetReason(OperatingSystemTypes::ResetReason &resetReason) = 0;
    /**
     * @brief Get the system MAC address.
     * @details Useful if you want to have access to a consistent MAC address if you're using multiple network interfaces.
     * @param[out] macAddress The system MAC address.
     * @returns ErrorType::Success if the system MAC address was obtained
     * @returns ErrorType::NotImplemented if getting the system MAC address is not implemented
     * @returns ErrorType::Failure if the system MAC address could not be obtained
     */
    virtual ErrorType getSystemMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) = 0;
    /**
     * @brief Perform a soft reset of the processor.
     * @returns ErrorType::Success if the processor was reset
     * @returns ErrorType::Failure if the processor could not be reset
     * @returns ErrorType::NotImplemented if resetting the processor is not implemented
     * @returns ErrorType::NotAvailable if resetting the processor is not available
    */
    virtual ErrorType reset() = 0;
    /**
     * @brief setTimeOfDay
     * @param[in] utc The time of day in unix time.
     * @param[in] timeZoneDifferenceUtc The time zone difference.
     * @returns ErrorType::Success if the time of day was set.
     * @returns ErrorType::NotImplemented if setting the time of day is not implemented.
     * @returns ErrorType::NotAvailable if setting the time of day is not available on the host system.
     * @returns ErrorType::Failure if an error occurred while setting the time of day.
    */
    virtual ErrorType setTimeOfDay(const UnixTime utc, const int16_t timeZoneDifferenceUtc) = 0;
    /**
     * @brief Get the percentage of time that the Operating system is idle.
     * @details Differs for Darwin and Linux. This percentage is how often we used the CPU vs how much we've been running.
     * @param[out] idlePercent The percentage of time that the operating system was idle.
     * @returns ErrorType::Success if the the idleTime could be obtained.
     * @returns ErrorType::NotImplemented if setting the time of day is not implemented.
     * @returns ErrorType::NotAvailable if setting the time of day is not available on the host system.
     * @returns ErrorType::Failure if the idleTime could not be obtained.
     */
    virtual ErrorType idlePercentage(Percent &idlePercent) = 0;
    /**
     * @brief Get the memory usage for a specific memory region
     * @param[in,out] region The memory region to get usage for (name should be set, free will be updated)
     * @returns ErrorType::Success if the memory region usage was obtained
     * @returns ErrorType::NotImplemented if getting memory region usage is not implemented
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType memoryRegionUsage(OperatingSystemTypes::MemoryRegionInfo &region) = 0;
    /**
     * @brief The amount of time the system has been running for since the last reset.
     * @returns ErrorType::Success if the uptime was obtained.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType uptime(Seconds &uptime) = 0;
    /**
     * @brief Disable interrupts
     * @returns ErrorType::Success if the interrupts were successfully disabled
     * @returns ErrorType::NotAvailable if disabling interrupts is not permissable on the system (i.e. an operating system environment)
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType disableAllInterrupts() = 0;
    /**
     * @brief Enable interrupts
     * @returns ErrorType::Success if the interrupts were successfully enabled
     * @returns ErrorType::NotAvailable if disabling interrupts is not permissable on the system (i.e. an operating system environment)
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType enableAllInterrupts() = 0;
    /**
     * @brief Block the task who calls this function.
     * @returns ErrorType::Success if the task was blocked
     * @returns ErrorType::Failure if the task was not blocked
     * @returns ErrorType::LimitReached If the task was previously unblocked before it called block.
     * @post You should always check the return value of block for ErrorType::LimitReached. You may need to call block again if the task
     *       has had unbalanced calls to block and unblock. If ErrorType::LimitReached is returned twice in a row, the second time indicates
     *       that unblock was called before the task had a chance to block itself and the task can safely proceed.
     */
    virtual ErrorType block() = 0;
    /**
     * @brief Unblock a task
     * @param task The Id of the task to unblock
     * @returns ErrorType::Success if the task was unblocked or not blocked previously
     * @returns ErrorType::NoData if the task ID was not found.
     * @returns ErrorType::Failure if the task was not unblocked
     */
    virtual ErrorType unblock(const Id task) = 0;
    /**
     * @brief Get the status of the operatings system as a const reference.
     * @returns The status of the operating system.
    */
    const OperatingSystemTypes::Status &status(bool updateStatus = false) {

        if (updateStatus) {
            getSystemTime(_status.systemTime);
            idlePercentage(_status.idle);
            uptime(_status.upTime);
            
            // Update memory region usage
            for (auto &memoryRegion : _status.memoryRegion) {
                memoryRegionUsage(memoryRegion);
            }
        }

        return _status;
    }

    /**
     * @brief Convert a software version string to a byte array
     * @param[in] softwareVersionString The software version string to convert
     * @param[out] softwareVersionByteArray The byte array to store the converted software version
     * @param[out] versionSize The size of the version string
     * @returns ErrorType::Success if the software version string was converted to a byte array
     * @returns ErrorType::Failure otherwise
     * @post Non-digit characters are discarded. The array represents a.b.c.d as {a, b, c, d}
     */
    template <size_t size>
    ErrorType softwareVersionStringToByteArray(std::string_view softwareVersionString, std::array<uint8_t, size> &softwareVersionByteArray, Bytes &versionSize) {
        std::fill(softwareVersionByteArray.begin(), softwareVersionByteArray.end(), 0);
        auto itr = softwareVersionString.begin();

        char currentlyReadingVersion = 'a';
        while(itr != softwareVersionString.end()) {
            if(*itr == '.') {
                itr++;
                currentlyReadingVersion++;
                continue;
            }

            assert(static_cast<Bytes>(currentlyReadingVersion - 'a') < size);

            softwareVersionByteArray[currentlyReadingVersion - 'a'] *= 10;
            const char digit = {*itr};
            softwareVersionByteArray[currentlyReadingVersion - 'a'] += strtoul(&digit, nullptr, 10);

            itr++;
        }

        return ErrorType::Success;
    }

    protected:
    /// @brief The status of the operating system
    OperatingSystemTypes::Status _status = {
        .threadCount = 0,
        .idle = -1,
        .upTime = 0,
        .memoryRegion = {},
        .systemTime = 0
    };

};

#endif //__OPERATING_SYSTEM_ABSTRACTION_HPP__