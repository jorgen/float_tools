#include "catch.hpp"

#include <stdint.h>

#include <stdio.h>
#include <limits>

#include <random>

#include <float_tools.h>

#include <thread>

#include <fmt/printf.h>

template<typename T>
void assert_char_length(T t, int expected_length)
{
  int length = ft::count_chars(t);
  REQUIRE(length == expected_length);
}
TEST_CASE("basic_count_chars", "[count_chars]")
{
  assert_char_length(UINT64_C(0), 1);
  assert_char_length(UINT64_C(1), 1);
  assert_char_length(UINT64_C(10), 2);
  assert_char_length(UINT64_C(100), 3);
  assert_char_length(UINT64_C(245), 3);
  assert_char_length(UINT64_C(1000), 4);
  assert_char_length(UINT64_C(10000), 5);
  assert_char_length(UINT64_C(100000), 6);
  assert_char_length(UINT64_C(1000000), 7);
  assert_char_length(UINT64_C(10000000), 8);
  assert_char_length(UINT64_C(100000000), 9);
  assert_char_length(UINT64_C(1000000000), 10);
  assert_char_length(UINT64_C(10000000000), 11);
  assert_char_length(UINT64_C(100000000000), 12);
  assert_char_length(UINT64_C(1000000000000), 13);
  assert_char_length(UINT64_C(10000000000000), 14);
  assert_char_length(UINT64_C(100000000000000), 15);
  assert_char_length(UINT64_C(1000000000000000), 16);
  assert_char_length(UINT64_C(10000000000000000), 17);
  assert_char_length(UINT64_C(100000000000000000), 18);
  assert_char_length(UINT64_C(567646353264743563), 18);
  assert_char_length(UINT64_C(1000000000000000000), 19);
  assert_char_length(UINT64_C(10000000000000000000), 20);
  
  assert_char_length(INT64_C(0), 1);
  assert_char_length(INT64_C(1), 1);
  assert_char_length(INT64_C(10), 2);
  assert_char_length(INT64_C(100), 3);
  assert_char_length(INT64_C(1000), 4);
  assert_char_length(INT64_C(10000), 5);
  assert_char_length(INT64_C(100000), 6);
  assert_char_length(INT64_C(1000000), 7);
  assert_char_length(INT64_C(10000000), 8);
  assert_char_length(INT64_C(100000000), 9);
  assert_char_length(INT64_C(1000000000), 10);
  assert_char_length(INT64_C(10000000000), 11);
  assert_char_length(INT64_C(100000000000), 12);
  assert_char_length(INT64_C(1000000000000), 13);
  assert_char_length(INT64_C(10000000000000), 14);
  assert_char_length(INT64_C(100000000000000), 15);
  assert_char_length(INT64_C(1000000000000000), 16);
  assert_char_length(INT64_C(10000000000000000), 17);
  assert_char_length(INT64_C(100000000000000000), 18);
  assert_char_length(INT64_C(1000000000000000000), 19);
  
  assert_char_length(UINT32_C(0), 1);
  assert_char_length(UINT32_C(1), 1);
  assert_char_length(UINT32_C(10), 2);
  assert_char_length(UINT32_C(100), 3);
  assert_char_length(UINT32_C(1000), 4);
  assert_char_length(UINT32_C(10000), 5);
  assert_char_length(UINT32_C(100000), 6);
  assert_char_length(UINT32_C(1000000), 7);
  assert_char_length(UINT32_C(10000000), 8);
  assert_char_length(UINT32_C(100000000), 9);
  assert_char_length(UINT32_C(1000000000), 10);
  
  assert_char_length(INT32_C(0), 1);
  assert_char_length(INT32_C(1), 1);
  assert_char_length(INT32_C(10), 2);
  assert_char_length(INT32_C(100), 3);
  assert_char_length(INT32_C(1000), 4);
  assert_char_length(INT32_C(10000), 5);
  assert_char_length(INT32_C(100000), 6);
  assert_char_length(INT32_C(1000000), 7);
  assert_char_length(INT32_C(10000000), 8);
  assert_char_length(INT32_C(100000000), 9);
  assert_char_length(INT32_C(1000000000), 10);
  
  assert_char_length(UINT16_C(0), 1);
  assert_char_length(UINT16_C(1), 1);
  assert_char_length(UINT16_C(10), 2);
  assert_char_length(UINT16_C(100), 3);
  assert_char_length(UINT16_C(1000), 4);
  assert_char_length(UINT16_C(10000), 5);

  assert_char_length(INT16_C(0), 1);
  assert_char_length(INT16_C(1), 1);
  assert_char_length(INT16_C(10), 2);
  assert_char_length(INT16_C(100), 3);
  assert_char_length(INT16_C(1000), 4);
  assert_char_length(INT16_C(10000), 5);
  
  assert_char_length(UINT8_C(0), 1);
  assert_char_length(UINT8_C(1), 1);
  assert_char_length(UINT8_C(10), 2);
  assert_char_length(UINT8_C(100), 3);
  
  assert_char_length(INT8_C(0), 1);
  assert_char_length(INT8_C(1), 1);
  assert_char_length(INT8_C(10), 2);
  assert_char_length(INT8_C(100), 3);
}
