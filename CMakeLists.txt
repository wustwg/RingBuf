cmake_minimum_required(VERSION 3.17)
project(ringbuf)

set(CMAKE_CXX_STANDARD 14)

add_executable(ringbuf main.cpp RingBuf.cpp RingBuf.h)
target_link_libraries(ringbuf PUBLIC pthread)