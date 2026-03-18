/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   OperatingSystemAbstraction.hpp
* @details Types for the operating system.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __OPERATING_SYSTEM_TYPES_HPP__
#define __OPERATING_SYSTEM_TYPES_HPP__

//AbstractionLayer
#include <Types.hpp>
//C++
#include <vector>

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

#endif //__OPERATING_SYSTEM_TYPES_HPP__