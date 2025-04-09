################################################################################
#Date: September 26th, 2023                                                    #
#File: CMakeLists.txt                                                          #
#Authour: Ben Haubrich                                                         #
#Synopsis: Top level CMakeList.txt.                                            # 
################################################################################
cmake_minimum_required(VERSION 3.5)

#Use cmake '--help-policy <POLICY>` to learn more
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)

add_library(abstractionLayer INTERFACE)

add_subdirectory(Abstractions/Cryptography)
add_subdirectory(Abstractions/Hardware)
add_subdirectory(Abstractions/Ip)
add_subdirectory(Abstractions/Logging)
add_subdirectory(Abstractions/Network)
add_subdirectory(Abstractions/OperatingSystem)
add_subdirectory(Abstractions/Protocols)
add_subdirectory(Abstractions/Serialization)
add_subdirectory(Abstractions/Storage)
