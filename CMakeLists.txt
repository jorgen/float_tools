cmake_minimum_required (VERSION 3.0)
project (float_to_string)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_EXTENSIONS OFF)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	add_compile_options(/permissive-)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-old-style-cast -Wno-shadow-field-in-constructor -Wno-switch-enum -Wno-padded -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic")
endif()
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
