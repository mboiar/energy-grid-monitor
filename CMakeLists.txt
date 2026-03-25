cmake_minimum_required(VERSION 3.15)

set(NAME StationC3)

project(${NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O2 -Wall")

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)

add_library(Boost::mqtt5 INTERFACE IMPORTED GLOBAL)
target_include_directories(Boost::mqtt5 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/dep/mqtt5/include
)

target_link_libraries(Boost::mqtt5 INTERFACE
    Boost::system
    Boost::thread
)

# For real-time scheduling
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lrt")

add_executable(${NAME}
    src/main.cpp
)

target_link_libraries(${NAME}
    Boost::system
    Boost::thread
    Boost::mqtt5
    pthread
    rt
    curl
)

target_compile_definitions(${NAME} PRIVATE SCHED_FIFO)

# Include directories
target_include_directories(${NAME}
    PRIVATE
    src
)