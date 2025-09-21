cmake_minimum_required(VERSION 3.20)
project(WSocket)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    set(ASIO_ROOT "I:/asio-1.36.0")
    set(ZSTD_ROOT "I:/zstd-v1.5.7")
else ()
    set(ASIO_ROOT "/mnt/i/asio-1.36.0")
endif ()

# asio
set(ASIO_INCLUDE_DIR "${ASIO_ROOT}/include")

# zstd
find_package(ZSTD REQUIRED)
if (ZSTD_FOUND)
    message(STATUS "zstd found")
    add_definitions(
            -DWITH_ZSTD
    )
    link_directories(
            ${ZSTD_ROOT}/lib
    )
    link_libraries(
            zstd_static
    )
endif ()

add_executable(WSocket
        main.cpp
        include/WSocketContext.hpp
)
target_include_directories(
        WSocket
        PRIVATE ${ASIO_INCLUDE_DIR}
        PRIVATE ${ZSTD_ROOT}/include
)
target_compile_definitions(
        WSocket
        PRIVATE WITH_ASIO
)

