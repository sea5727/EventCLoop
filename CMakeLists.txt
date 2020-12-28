cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

link_libraries(pthread)

add_executable(test1_epoll test1_epoll.cpp)
# add_executable(test1_server test1_server.cpp)
# add_executable(test2_client test2_client.cpp)
# add_executable(test3_myserver test3_myserver.cpp)
# add_executable(test4_myclient test4_myclient.cpp)

add_subdirectory(Doc/epoll)
add_subdirectory(Doc/timerfd)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
