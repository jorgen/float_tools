cmake_minimum_required (VERSION 3.0)
project (float_to_string)

set (CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

include (cmake/Fetch3rdParty.cmake)
Fetch3rdParty()

include (cmake/BuildFmt.cmake)
BuildFmt()

#include (cmake/pandocology.cmake)

set(TTMATH_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3rdparty/ttmath")
set(TTMATH_DEFINES "TTMATH_NOASM=1;TTMATH_PLATFORM64=1")

add_subdirectory(tools)

#add_subdirectory(docs)

enable_testing()
include(cmake/Catch.cmake)
add_subdirectory(tests)
