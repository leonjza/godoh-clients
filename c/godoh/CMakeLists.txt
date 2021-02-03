cmake_minimum_required(VERSION 3.10)
project(godoh C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall -Wextra")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-Wall -Wextra")
if (NOT WIN32)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -g")
endif ()


option(DEBUG "enable debug mode." OFF)
if (DEBUG)
    add_definitions(-DDEBUG)
endif (DEBUG)

file(GLOB_RECURSE SOURCES RELATIVE
        ${CMAKE_SOURCE_DIR}
        "src/**.c"
        "src/**.h"
        )

add_executable(godoh ${SOURCES})

if (WIN32)
    target_link_libraries(godoh dnsapi.dll)
    target_link_libraries(godoh ws2_32)
ELSE ()
    target_link_libraries(godoh z)
    target_link_libraries(godoh resolv)
ENDIF ()


if (DOMAIN)
    add_definitions(-DDOMAIN="${DOMAIN}")
else ()
    add_definitions(-DDOMAIN="c2.test")
endif ()

if (MAX_SLEEP)
    add_definitions(-DMAX_SLEEP=${MAX_SLEEP})
endif ()

if (CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} ${PROJECT_NAME})
endif ()
