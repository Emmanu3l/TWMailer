cmake_minimum_required(VERSION 3.12)
project(TWMailer)

set(CMAKE_CXX_STANDARD 17)

add_executable(myserver myserver.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(myserver Threads::Threads)