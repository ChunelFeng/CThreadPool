message("* * * * * * * * * * * * * * * * *")
message("*   _____    _______    _____   *")
message("*  / ____|  |__   __|  |  __ \\  *")
message("* | |          | |     | |__) | *")
message("* | |          | |     |  ___/  *")
message("* | |____      | |     | |      *")
message("*  \\_____|     |_|     |_|      *")
message("* * * * * * * * * * * * * * * * *")

cmake_minimum_required(VERSION 3.5.0)

project(CThreadPool VERSION 1.3.0)

set(CMAKE_CXX_STANDARD 11)

# add CThreadPool environment info
include(cmake/CThreadPool-env-include.cmake)

# 如果开启此宏定义，则CGraph执行过程中，不会在控制台打印任何信息
# add_definitions(-D_CGRAPH_SILENCE_)

# 编译libCThreadPool动态库
# add_library(CThreadPool SHARED ${CTP_SRC_LIST})

# 编译libCThreadPool静态库
# add_library(CThreadPool STATIC ${CTP_SRC_LIST})

add_executable(tutorial
        ${CTP_SRC_LIST}
        tutorial.cpp)
