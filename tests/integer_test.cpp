#include <float_tools.h>

#include "catch.hpp"

#include <string>

#define TEST_INT(x) test_int(x, #x)
template<typename T>
void test_int(T i, const std::string &valid_str)
{
  std::string str;
  str.resize(ft::StaticLog10<T, std::numeric_limits<T>::max(), 0, 0, true>::get() + 1);
  int truncated;
  str.resize(ft::integer::to_buffer(i, &str[0], str.size(), &truncated));
  REQUIRE(truncated == 0);
  REQUIRE(str == valid_str);
}

template<typename T>
void test_int2(const std::string& str, const T t)
{
  T result;
  const char* endptr;
  auto parse_result = ft::integer::to_integer(str, result, endptr);
  REQUIRE(parse_result == ft::parse_string_error::ok);
  REQUIRE(result == t);
}

TEST_CASE("basic_int_to_string", "[integer to string]")
{
  int low = 123;
  std::string lowStr;
  lowStr.resize(12);
  int truncated;
  lowStr.resize(ft::integer::to_buffer(low, &lowStr[0], lowStr.size(), &truncated));
  REQUIRE(lowStr == "123");

  TEST_INT(123);
  TEST_INT(0);
  test_int(std::numeric_limits<uint8_t>::max(), "255");
  test_int(std::numeric_limits<uint16_t>::max(), "65535");
  test_int(std::numeric_limits<uint32_t>::max(), "4294967295");
  test_int(std::numeric_limits<uint64_t>::max(), "18446744073709551615");
  test_int(std::numeric_limits<int8_t>::max(), "127");
  test_int(std::numeric_limits<int8_t>::min(), "-128");
  test_int(std::numeric_limits<int16_t>::max(), "32767");
  test_int(std::numeric_limits<int16_t>::min(), "-32768");
  test_int(std::numeric_limits<int32_t>::max(), "2147483647");
  test_int(std::numeric_limits<int32_t>::min(), "-2147483648");
  test_int(std::numeric_limits<int64_t>::max(), "9223372036854775807");
  test_int(std::numeric_limits<int64_t>::min(), "-9223372036854775808");
}

TEST_CASE("basic_string_to_int", "[string to integer]")
{
  test_int2("255", std::numeric_limits<uint8_t>::max());
  test_int2("65535", std::numeric_limits<uint16_t>::max());
  test_int2("4294967295", std::numeric_limits<uint32_t>::max());
  test_int2("18446744073709551615", std::numeric_limits<uint64_t>::max());
  test_int2("127", std::numeric_limits<int8_t>::max());
  test_int2("-128", std::numeric_limits<int8_t>::min());
  test_int2("32767", std::numeric_limits<int16_t>::max());
  test_int2("-32768", std::numeric_limits<int16_t>::min());
  test_int2("2147483647", std::numeric_limits<int32_t>::max());
  test_int2("-2147483648", std::numeric_limits<int32_t>::min());
  test_int2("9223372036854775807", std::numeric_limits<int64_t>::max());
  test_int2("-9223372036854775808", std::numeric_limits<int64_t>::min());
  
  test_int2("-9223372036854775808e-10", INT64_C(-922337203));
  test_int2("12.567e8", 1256700000);
  test_int2("9223372036854775807e-100", UINT64_C(0));
  test_int2("-9223372036854775808e-100", INT64_C(0));

  test_int2("4294967295e30", std::numeric_limits<uint32_t>::max());
  test_int2("18446744073709551615e10", std::numeric_limits<uint64_t>::max());
}
