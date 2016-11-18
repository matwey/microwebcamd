cmake_minimum_required (VERSION 2.6)
project(microwebcamd C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CheckSymbolExists)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(CHECK REQUIRED check)
pkg_check_modules(ATOMIC_OPS REQUIRED atomic_ops)
pkg_check_modules(SYSTEMD libsystemd)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_C_FLAGS "-std=c99 -O3 -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=600 ${MICROHTTPD_CFLAGS} ${CHECK_CFLAGS} ${SYSTEMD_CFLAGS}")

include_directories ("${PROJECT_SOURCE_DIR}/include" ${MICROHTTPD_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS} ${SYSTEMD_INCLUDE_DIRS} ${ATOMIC_OPS_INCLUDE_DIRS})

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MICROHTTPD_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${MICROHTTPD_LIBRARIES})

check_symbol_exists(epoll_create1 "sys/epoll.h" HAVE_EPOLL_CREATE1)

if(SYSTEMD_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_SYSTEMD")
endif(SYSTEMD_FOUND)

file(GLOB_RECURSE SOURCES src/*.c)
add_executable(microwebcamd ${SOURCES})
target_link_libraries(microwebcamd ${MICROHTTPD_LIBRARIES} ${SYSTEMD_LIBRARIES} ${ATOMIC_OPS_LIBRARIES} Threads::Threads)
install(TARGETS microwebcamd DESTINATION bin)

enable_testing()
add_executable(test_list test/list.c src/list.c)
target_link_libraries(test_list ${CHECK_LIBRARIES})
add_test(test_list test_list)
