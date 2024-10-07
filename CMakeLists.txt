cmake_minimum_required(VERSION 3.19)
project(miniLog)

set(CMAKE_CXX_STANDARD 11)

add_executable(miniLog main.cpp Log/miniLog.h Log/miniLog.cpp Log/SpinLock.h)