/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   MemoryPool.hpp
* @details \b Synopsis: \n Statically allocate memory and then allocate from it at run time in configurable blocks.
* @see https://en.wikipedia.org/wiki/Memory_pool
* @ingroup Applications
*******************************************************************************/
#ifndef __MEMORY_POOL_HPP__
#define __MEMORY_POOL_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <array> //Some of the compilers that were tested did not have <span>
#include <cstddef>

/**
 * @class MemoryPool
 * @brief Statically allocate memory and then allocate memory from it at runtime as if it were dynamically allocated.
 * @details If we get carried away with features and trying to manage fragementation, we'll just end up implementing a heap allocator.
 *          The mempool is meant to be simple and naive so that it saves time in comparision to a heap allocator. If you are using a mempool,
 *          you know ahead of time what the max size of the allocations will be and the max number of items that will could be allocated at once.
 *          If you do not know this then a mempool is not a good idea.
 * @pre _poolSize must be a multiple of sizeof(T) otherwise you're just wasting memory!
 * @tparam T The type of the memory pool.
 * @tparam _poolSize The size of the pool.
 * @details We want to be able to initialize this at compile time so that's what the template is for.
 * @attention This class is not threadsafe. It is not reccomended to use the same pool accross multiple threads.
 */
template<typename T, Bytes _poolSize>
class MemoryPool {

    public:
    /// @brief Default constructor
    constexpr MemoryPool() {
        static_assert(_poolSize % sizeof(T) == 0, "Pool size should be a multiple of the block size");
    }

    /// @brief Return the size of the memory pool
    constexpr Bytes poolSize() const { return _poolSize; }
    /// @brief Return the size of a block in the memory pool
    constexpr Bytes blockSize() const { return sizeof(T); }

    /**
     * @brief Allocate memory from the pool.
     * @param[out] poolBlock The pointer to the block of memory allocated from the pool.
     * @return ErrorType::Success if the memory was allocated
     * @returns ErrorType::NoMemory if the memory was not allocated.
     * @sa setData for a safe way to set the newly allocated block
     */ 
    ErrorType allocate(T *&poolBlock) {
        for (size_t i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                poolBlock = &_pool[i];
                return ErrorType::Success;
            }
        }

        return ErrorType::NoMemory;
    }

    /**
     * @brief Deallocate memory from the pool.
     * @param[in] poolBlock The pointer to the block of memory to deallocate.
     * @return ErrorType::Success if the memory was deallocated
     * @returns ErrorType::InvalidParameter if the memory was not deallocated.
     */
    ErrorType deallocate(T *&poolBlock) {
        for (size_t i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (&_pool[i] == poolBlock) {
                _blockAllocationMap[i] = 0;
                return ErrorType::Success;
            }
        }

        return ErrorType::InvalidParameter;
    }

    /**
     * @brief Set the data in a memory block.
     * @param[in] poolBlock The pointer to the block of memory to set the data in.
     * @param[in] data The data to set in the memory block.
     * @param[in] dataSize The size of the data to set in the memory block.
     * @return ErrorType::Success if the data was set
     * @returns ErrorType::InvalidParameter if the data is too large to fit in the block.
     * @returns ErrorType::InvalidParameter if the data being set does not belong to the pool.
     */
    ErrorType setData(T *&poolBlock, const T *data, Bytes dataSize) {
        const bool dataIsTooLarger = (dataSize > sizeof(T));
        const bool dataDoesNotBelongToPool = (data < &_pool[0] || data >= &_pool[sizeof(_pool) - 1]);
        if (dataIsTooLarger) {
            return ErrorType::InvalidParameter;
        }
        else if (dataDoesNotBelongToPool) {
            return ErrorType::InvalidParameter;
        }

        std::copy(data, data + dataSize, poolBlock);
        return ErrorType::Success;
    }

    /**
     * @brief Get the available memory in the pool.
     * @param[out] size The size of the available memory in the pool.
     * @return ErrorType::Success always
     */
    ErrorType available(Bytes &size) const {
        size = 0;
        for (size_t i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                size += sizeof(T);
            }
        }

        return ErrorType::Success;
    }

    private:
    /// @brief The pool of memory
    std::array<T, _poolSize> _pool;
    /// @brief A map of which blocks are available.
    uint8_t _blockAllocationMap[_poolSize / sizeof(T)] = {0};

    /**
     * @brief Check if a block is available.
     * @param[in] i The index of the block to check.
     * @return true if the block is available.
     * @return false if the block is not available.
     */
    bool blockIsAvailable(const size_t i) const {
        constexpr int blockMarkedAvailable = 0;
        return _blockAllocationMap[i] == blockMarkedAvailable;
    };
};

#endif // __MEMORY_POOL_HPP__