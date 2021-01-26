cmake_minimum_required(VERSION 3.17)
project(godoh C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall -Wextra")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -g")


option(DEBUG "enable debug mode." OFF)
if (DEBUG)
    add_definitions(-DDEBUG)
endif(DEBUG)

file(GLOB_RECURSE SOURCES RELATIVE
        ${CMAKE_SOURCE_DIR}
        "src/**.c"
        "src/**.h"
)

add_executable(godoh ${SOURCES})

target_link_libraries(godoh z)
target_link_libraries(godoh resolv)

if (DOMAIN)
    add_compile_definitions(godoh PRIVATE "DOMAIN=\"${DOMAIN}\"")
else()
    add_compile_definitions(godoh PRIVATE "DOMAIN=\"c2.test\"")
endif()
