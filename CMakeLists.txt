cmake_minimum_required(VERSION 3.20)
project(WSocket)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    set(ASIO_ROOT "I:/asio-1.36.0")
else ()
    set(ASIO_ROOT "/mnt/i/asio-1.36.0")
endif ()
set(ASIO_INCLUDE_DIR "${ASIO_ROOT}/include")

add_executable(WSocket
        main.cpp
        include/WSocketContext.hpp
)
target_include_directories(
        WSocket
        PRIVATE ${ASIO_INCLUDE_DIR}
)
target_compile_definitions(
        WSocket
        PRIVATE WITH_ASIO
)
