cmake_minimum_required(VERSION 3.10)

# Set the project name
project(StockDataDownloader)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find CURL
find_package(CURL REQUIRED)

# Add executable
add_executable(main main.cpp)

# Link CURL library
target_link_libraries(main CURL::libcurl)
