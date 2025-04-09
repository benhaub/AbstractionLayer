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

namespace Algorithm {
    /**
     * @brief Splits a string into a vector of strings by a delimiter
     * @param s The string to split
     * @param delimiter The delimiter to split the string by
     * @return std::vector<std::string> The vector of strings
     * @author https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
     */
    std::vector<std::string> split(std::string& s, const char delimiter[]) {
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
}

#endif //__ALGORITHM_HPP__