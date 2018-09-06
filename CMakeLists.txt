cmake_minimum_required (VERSION 3.11)

project(iss-receiver)
set(CMAKE_CXX_STANDARD 17)

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "-ggdb -Wall -Wextra -Werror -Wno-implicit-fallthrough ${CMAKE_CXX_FLAGS}")
endif()

add_library(receiver
    IReceiver.h
    Receiver.h
    Receiver.cpp
)

add_executable(tests
    TestRunner.cpp
    Tests.cpp
)

target_link_libraries(tests receiver)
