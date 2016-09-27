cmake_minimum_required (VERSION 2.6)
project(microwebcamd C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CheckSymbolExists)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(CHECK REQUIRED check)
pkg_check_modules(SYSTEMD libsystemd)
find_package(CURL REQUIRED)

set(CMAKE_C_FLAGS "-std=c99 -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED ${MICROHTTPD_CFLAGS} ${CHECK_CFLAGS} ${SYSTEMD_CFLAGS}")

include_directories ("${PROJECT_SOURCE_DIR}/include" ${MICROHTTPD_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS} ${SYSTEMD_INCLUDE_DIRS})

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MICROHTTPD_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${MICROHTTPD_LIBRARIES})

if(SYSTEMD_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_SYSTEMD")
endif(SYSTEMD_FOUND)

file(GLOB_RECURSE SOURCES src/*.c)
add_executable(microwebcamd ${SOURCES})
target_link_libraries(microwebcamd ${MICROHTTPD_LIBRARIES} ${SYSTEMD_LIBRARIES})
install(TARGETS microwebcamd DESTINATION bin)

enable_testing()
add_executable(test_list test/list.c src/list.c)
target_link_libraries(test_list ${CHECK_LIBRARIES})
add_test(test_list test_list)
