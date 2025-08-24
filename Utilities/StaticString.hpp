/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   StaticString.hpp
* @details Wrapper for boost::static_string that is not templated so it does not need a fixed size type.
* @ingroup Utilities
*******************************************************************************/
#ifndef __FIXED_STRING_HPP__
#define __FIXED_STRING_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
//Boost
#include "boost/static_string/config.hpp"
#include "boost/static_string.hpp"
//C++
#include <any>

/**
 * @namespace StaticString
 * @details Allows a templated string to be stored in a unified type, regardless of it's size.
 */
namespace StaticString {

    /**
     * @class StandardStringInterface
     * @brief Interface for implementing std::string-like strings.
     */
    class StandardStringInterface {

        public:
        virtual ~StandardStringInterface() = default;
        /// @brief Implementation defined.
        virtual const char *c_str() const = 0;
        /// @brief Implementation defined.
        virtual size_t size() const = 0;
        /// @brief Implementation defined.
        virtual size_t length() const { return size(); }
        /// @brief Implementation defined.
        virtual char* data() = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface& append(const char *s) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface& append(std::string_view) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface& append(const char *s, size_t startPosition, size_t length) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface& assign(const char *s) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface& assign(std::string_view) = 0;
        /// @brief Implementation defined.
        virtual StandardStringInterface& assign(const char *s, size_t startPosition, size_t length = 0) = 0;
    };

    /**
     * @class Data
     * @brief Implements the StandardStringInterface using static strings.
     * @tparam _n The size of the static string.
     */
    template<size_t _n>
    class Data final : public StandardStringInterface {

        public:
        /**
         * @brief Constructor.
         * @param s The string to initialize the static string to.
         */
        Data(const char* s) : _str(s) {}

        /// @brief The underlying static string.
        boost::static_string<_n> _str;

        const char *c_str() const override { return _str.c_str(); }
        size_t size() const override { return _str.size(); }
        char *data() { return _str.data(); }
        StandardStringInterface &append(const char *s) { _str.append(s); return *this; }
        StandardStringInterface &append(std::string_view s) { _str.append(s); return *this; }
        StandardStringInterface &append(const char *s, size_t startPosition, size_t length) override {
            const auto first = _str.begin() + startPosition;
            const auto last = _str.begin() + length;
            _str.append(first, last); return *this;
        }
        StandardStringInterface &assign(const char *s) override {
            _str.assign(s); return *this;
        }
        StandardStringInterface &assign(std::string_view s) override {
            _str.assign(s); return *this;
        }
        StandardStringInterface &assign(const char *s, size_t startPosition, size_t length = 0) override {
            if (0 == length) {
                _str.assign(s + startPosition);
            }
            else {
                const auto first = _str.begin() + startPosition;
                const auto last = _str.begin() + length;
                _str.assign(first, last);
            }

            return *this;
        }
    };

    /**
     * @class Container
     * @brief Template type-erasure for anything that inherits from the Interface type.
     * @details This is useful for storing template classes in a common container without needing to know the template parameters
     */
    class Container {

        private:
        /// @brief The pure virtual interface that the container returns.
        using Interface = StandardStringInterface;
        /// @brief The copy of the static string data.
        std::any _data;
        /**
         * @brief A pointer so that the we any_cast to a Interface pointer.
         * @details A cast directly from T to Interface* is not allowed, so we need this intermediate step.
         */
        std::any _dataPtr;
        /// @brief Same as _dataPtr, but for const pointers.
        std::any _dataPtrConst;

        public:
        /**
         * @brief Set the contained string to a specific type.
         * @tparam T The type of static string to store.
         * @param value The value to set the string to.
         */
        template<typename T>
        requires std::is_base_of_v<Interface, T>
        void set(const T& value) {
            _data = value;
            T *dataPtr = std::any_cast<T>(&_data);
            _dataPtr = static_cast<Interface *>(dataPtr);
            const T *dataPtrConst = std::any_cast<const T>(&_data);
            _dataPtrConst = static_cast<const Interface *>(dataPtrConst);
        }

        /// @brief Get a constant interface pointer
        const Interface *const getConst() const {
            assert(_data.has_value());
            const auto ptr = std::any_cast<const Interface *>(_dataPtrConst);
            return ptr;
        }
        /// @brief Get a mutable interface pointer
        Interface *get() {
            assert(_data.has_value());
            auto ptr = std::any_cast<Interface *>(_dataPtr);
            return ptr;
        }

        /// @brief Shorthand operator for Container::get
        Interface *operator->() {
            return get();
        }
        /// @brief Shorthand operator for Container::get
        const Interface *operator->() const {
            return getConst();
        }
    };
}

#endif //__FIXED_STRING_HPP__