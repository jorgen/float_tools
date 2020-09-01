include_directories(${PROJECT_SOURCE_DIR}/include)
include(CTest)
add_executable(basic_test
    catch_main.cpp
    basic_test.cpp
    count_chars_test.cpp
    )
target_link_libraries(basic_test fmt::fmt)
target_include_directories(basic_test PRIVATE ${TTMATH_INCLUDE_DIR})
target_compile_definitions(basic_test PRIVATE ${TTMATH_DEFINES})
add_dependencies(basic_test generate_ryu_cache)
catch_discover_tests(basic_test)
#add_test(NAME basic_test COMMAND basic_test)
