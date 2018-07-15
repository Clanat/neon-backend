cmake_minimum_required(VERSION 3.10)
project(neon_backend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "-d -m32 -ldl -lzmq -lproto -fpic -shared -g")
set(CMAKE_CXX_FLAGS "-m32 -g")

find_package(Protobuf REQUIRED)
find_package(ZMQ REQUIRED)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS source/protobuf/Models.proto)

include_directories(${CMAKE_BINARY_DIR})
add_library(neon_backend SHARED
        ${PROTO_SRCS}
        ${PROTO_HDRS}
        source/glx_hook.c
        source/APIService.cpp
        source/APIService.h source/MemoryService.cpp source/MemoryService.h source/FunctionService.cpp source/FunctionService.h)

set_target_properties(neon_backend PROPERTIES COMPILE_FLAGS "-m32 -g" LINK_FLAGS "-m32 -g")


target_include_directories(neon_backend PUBLIC source)


target_link_libraries(neon_backend PUBLIC
        "/usr/lib32/libGL.so"
        "/usr/lib/i386-linux-gnu/libzmq.so"
        "/usr/lib32/libdl.so"
        "/usr/lib/i386-linux-gnu/libprotobuf.so"
)