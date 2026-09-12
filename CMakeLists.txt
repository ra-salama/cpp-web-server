cmake_minimum_required(VERSION 3.14)

# Define project name and language
project(cpp_web_server VERSION 1.0 LANGUAGES CXX)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable common compiler warnings for clean code practice
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Define the executable output from main.cpp and Server.cpp
add_executable(server main.cpp Server.cpp HttpRequest.cpp)