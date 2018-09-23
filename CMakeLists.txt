cmake_minimum_required(VERSION 3.12)
project(SteelSeriesControl)

include_directories(SYSTEM /usr/include)
include_directories(SYSTEM /usr/lib/gcc/x86_64-linux-gnu/7/include)
set(CMAKE_INCLUDE_PATH /usr/include)
#set(CMAKE_INCLUDE_PATH /usr/lib/gcc/x86_64-linux-gnu/7/include)


link_directories(/usr/lib)

set(CMAKE_CXX_STANDARD 17)

add_executable(SteelSeriesControl main.cpp)

target_link_libraries(SteelSeriesControl /usr/lib/x86_64-linux-gnu/libusb-1.0.so)
