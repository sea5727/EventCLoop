cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

link_libraries(pthread)

add_executable(test1_epoll test1_epoll.cpp)
add_executable(test2_timer test2_timer.cpp)
add_executable(test3_client test3_client.cpp)
add_executable(test4_server test4_server.cpp)
add_executable(test6_signal test6_signal.cpp)


# add_executable(test5_nonblock_read test5_nonblock_read.cpp)
# add_executable(test5_nonblock_send test5_nonblock_send.cpp)


add_subdirectory(Doc/epoll)
add_subdirectory(Doc/timerfd)
add_subdirectory(Doc/socket)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
