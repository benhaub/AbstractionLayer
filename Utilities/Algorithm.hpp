/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Algorithm.hpp
* @details \b Synopsis: \n Algorithms
* @ingroup Utilities
*******************************************************************************/
#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

//C++
#include <vector>
#include <string>
#include <cstring>
#include <string_view>
#include <array>
#include <algorithm>

namespace Algorithm {
    /**
     * @brief Splits a string into a vector of strings by a delimiter
     * @param s The string to split
     * @param delimiter The delimiter to split the string by
     * @return std::vector<std::string> The vector of strings
     * @author https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
     */
    inline std::vector<std::string> Split(std::string& s, const char delimiter[]) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + strlen(delimiter));
        }
        tokens.push_back(s);

        return tokens;
    }

    /**
     * @concept StringLike
     * @brief Concept for a string-like type
     * @tparam T The type to check
     * @returns True if the type is string-like, false otherwise
     */
    template<typename T>
    concept StringLike = requires(T t) {
#if _GLIBCXX_CONCEPTS || _LIBCPP_STD_VER >= 20
        { t.data() } -> std::convertible_to<const char*>;
        { t.size() } -> std::convertible_to<size_t>;
        { t.begin() } -> std::contiguous_iterator;
        { t.end() } -> std::contiguous_iterator;
#else
        { true };
#endif
    };

    /**
     * @brief Compile-time string concatenation function
     * @param strings The strings to concatenate
     * @return std::array<char, total_size + 1> The concatenated string
     * @code{.cpp}
     * constexpr auto ConcatString = ConcatenateStrings<String1.size(), String2.size(), String3.size()>(
     *     String1, String2, String3
     * );
     * @endcode
     */
    template<StringLike... Args>
    inline constexpr auto ConcatenateStrings(Args... strings) {
        constexpr size_t totalSize = (strings.size() + ...);
        std::array<char, totalSize + 1> result = {};

        size_t pos = 0;
        ((std::copy_if(strings.begin(), strings.end(), result.begin() + pos, 
                       [](char c) { return c != '\0'; }), 
          pos += std::count_if(strings.begin(), strings.end(), [](char c) { return c != '\0'; })), ...);
        
        result[pos] = '\0';
        
        return result;
    }
}

#endif //__ALGORITHM_HPP__