#include "catch.hpp"

#include <stdint.h>

#include <stdio.h>
#include <limits>

#include <random>

#include <float_tools.h>

#include <thread>

#include <fmt/printf.h>

#define CHECK_FLOAT_STR(X) \
  do \
  { \
    std::string check_str = ft::ryu::to_string(float(X)); \
    REQUIRE(check_str == #X); \
  } while(0)

#define CHECK_DOUBLE_STR(X) \
  do \
  { \
    std::string check_str = ft::ryu::to_string(double(X)); \
    REQUIRE(check_str == #X); \
  } while(0)

#define CHECK_FLOAT_TRUNCATED(X, TO_TRUNCATE) \
  do { \
    std::string check; \
    check.resize((sizeof(#X) - 1) - TO_TRUNCATE); \
    int digits_truncated; \
    int size = ft::ryu::to_buffer(float(X), &check[0], check.size(), &digits_truncated); \
    std::string prefix(check.data(), check.data() + size); \
    REQUIRE(digits_truncated == TO_TRUNCATE); \
    REQUIRE(starts_with(std::string(#X), prefix)); \
  } while (0)

inline bool starts_with(const std::string &s, const std::string &prefix)
{
  return s.rfind(prefix, 0) == 0;
}

TEST_CASE("print_test_float", "[output_string]")
{
  CHECK_FLOAT_STR(0.123);
  CHECK_FLOAT_STR(-0.123);
  CHECK_FLOAT_STR(123.4567);
  CHECK_FLOAT_STR(-123.6543);
  CHECK_FLOAT_STR(0.0004567);
  CHECK_FLOAT_STR(-0.0006543);
  CHECK_FLOAT_STR(1234567.0);
  CHECK_FLOAT_STR(-1236543.0);
  CHECK_FLOAT_STR(1.234e23);
  CHECK_FLOAT_STR(1.234e-23);
  CHECK_DOUBLE_STR(0.000000000000101);
  //CHECK_DOUBLE_STR(4814913076773186048);
  //CHECK_DOUBLE_STR(4818058740581139328);

  CHECK_FLOAT_TRUNCATED(0.1234, 2);
  CHECK_FLOAT_TRUNCATED(0.1234, 4);
  CHECK_FLOAT_TRUNCATED(1234.1234, 4);
  CHECK_FLOAT_TRUNCATED(1234.1234, 2);
  CHECK_FLOAT_TRUNCATED(0.001, 3);
  CHECK_FLOAT_TRUNCATED(0.001, 4);
  CHECK_FLOAT_TRUNCATED(1.0, 1);


}
