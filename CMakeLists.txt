cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

ADD_COMPILE_OPTIONS( -Wall )
# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable -Wno-unused-parameter")

include(CTest)
enable_testing()

include_directories(${PROJECT_SOURCE_DIR})
link_libraries(pthread)

add_subdirectory(test)

add_subdirectory(Doc/epoll)
add_subdirectory(Doc/timerfd)
add_subdirectory(Doc/socket)
add_subdirectory(Doc/signal)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
