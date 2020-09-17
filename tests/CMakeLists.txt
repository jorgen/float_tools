include_directories(${PROJECT_SOURCE_DIR}/include)
include(CTest)
macro(create_test name)

  add_executable(${name}
    catch_main.cpp
    basic_test.cpp
    count_chars_test.cpp
    print_test.cpp
    integer_test.cpp
    )

  target_link_libraries(${name} fmt::fmt)
  target_include_directories(${name} PRIVATE ${TTMATH_INCLUDE_DIR})
  target_compile_definitions(${name} PRIVATE ${TTMATH_DEFINES})
  add_dependencies(${name} generate_ryu_cache)
  catch_discover_tests(${name})
endmacro()

create_test(default_test)
create_test(fast_math_test)
if (MSVC)
  target_compile_options(fast_math_test PUBLIC /fp:fast)
else()
  target_compile_options(fast_math_test PUBLIC -ffast-math)
endif()
#add_test(NAME basic_test COMMAND basic_test)
