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

/**
 * @class MemoryPool
 * @brief Statically allocate memory and then allocate memory from it at runtime as if it were dynamically allocated.
 * @details If we get carried away with features and trying to manage fragementation, we'll just end up implementing a heap allocator.
 *          The mempool is meant to be simple and naive so that it saves time in comparision to a heap allocator. If you are using a mempool,
 *          you know ahead of time what the max size of the allocations will be and the max number of items that will could be allocated at once.
 *          If you do not know this then a mempool is not a good idea.
 * @pre _poolSize must be a multiple of _blockSize otherwise you're just wasting memory!
 * @tparam _poolSize The size of the pool.
 * @tparam _blockSize The size of each block in the pool.
 * @details We want to be able to initialize this at compile time so that's what the template is for.
 * @attention This class is not threadsafe. It is not reccomended to use the same pool accross multiple threads.
 */
template<Bytes _poolSize, Bytes _blockSize>
class MemoryPool {

    public:
    /// @brief Default constructor
    constexpr MemoryPool() {
        static_assert(_poolSize % _blockSize == 0, "Pool size should be a multiple of the block size");
    }

    /// @brief Return the size of the memory pool
    constexpr Bytes poolSize() const { return _poolSize; }
    /// @brief Return the size of a block in the memory pool
    constexpr Bytes blockSize() const { return _blockSize; }

    /**
     * @brief Allocate memory from the pool.
     * @param[out] poolBlock The pointer to the block of memory allocated from the pool.
     * @return ErrorType::Success if the memory was allocated
     * @returns ErrorType::NoMemory if the memory was not allocated.
     * @sa setData for a safe way to set the newly allocated block
     */ 
    ErrorType allocate(uint8_t *&poolBlock) {
        for (int i = 0; i < sizeof(_blockAllocationMap); i++) {
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
    ErrorType deallocate(uint8_t *&poolBlock) {
        for (int i = 0; i < sizeof(_blockAllocationMap); i++) {
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
     */
    ErrorType setData(uint8_t *&poolBlock, const uint8_t *data, Bytes dataSize) {
        if (dataSize > _blockSize) {
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
        for (int i = 0; i < sizeof(_blockAllocationMap); i++) {
            if (blockIsAvailable(i)) {
                size += _blockSize;
            }
        }

        return ErrorType::Success;
    }

    private:
    /// @brief The pool of memory
    std::array<uint8_t, _poolSize> _pool;
    /// @brief A map of which blocks are available.
    uint8_t _blockAllocationMap[_poolSize / _blockSize] = {0};

    /**
     * @brief Check if a block is available.
     * @param[in] i The index of the block to check.
     * @return true if the block is available.
     * @return false if the block is not available.
     */
    ErrorType blockIsAvailable(int i) const {
        constexpr int blockMarkedAvailable = 0;
        return _blockAllocationMap[i] == blockMarkedAvailable;
    };
};

#endif // __MEMORY_POOL_HPP__