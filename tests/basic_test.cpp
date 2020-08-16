#include "catch.hpp"

#include <stdint.h>

#include <stdio.h>
#include <limits>

#include <random>

#include <float_tools.h>

#include <thread>

#include <fmt/printf.h>

TEST_CASE("small test", "[ryu]")
{
  std::string check_str = ft::ryu::to_string(1.0);
  REQUIRE(check_str == "1e0");
  check_str = ft::ryu::to_string(1.1);
  REQUIRE(check_str == "1.1e0");
  check_str = ft::ryu::to_string(22.1);
  REQUIRE(check_str == "2.21e1");
  check_str = ft::ryu::to_string(333.1);
  REQUIRE(check_str == "3.331e2");
  check_str = ft::ryu::to_string(22.4444);
  REQUIRE(check_str == "2.24444e1");
  check_str = ft::ryu::to_string(0.1);
  REQUIRE(check_str == "1e-1");
  check_str = ft::ryu::to_string(0.22);
  REQUIRE(check_str == "2.2e-1");
  check_str = ft::ryu::to_string(1.22);
  REQUIRE(check_str == "1.22e0");
  check_str = ft::ryu::to_string(22.22);
  REQUIRE(check_str == "2.222e1");
  check_str = ft::ryu::to_string(std::numeric_limits<double>::min());
  REQUIRE(check_str == "2.2250738585072013e-308");
  check_str = ft::ryu::to_string(std::numeric_limits<double>::max());
  REQUIRE(check_str == "1.7976931348623158e308");
}

TEST_CASE("small test negative", "[ryu]")
{
  std::string check_str = ft::ryu::to_string(-1.0);
  REQUIRE(check_str == "-1e0");
  check_str = ft::ryu::to_string(-1.1);
  REQUIRE(check_str == "-1.1e0");
  check_str = ft::ryu::to_string(-22.1);
  REQUIRE(check_str == "-2.21e1");
  check_str = ft::ryu::to_string(-333.1);
  REQUIRE(check_str == "-3.331e2");
  check_str = ft::ryu::to_string(-22.4444);
  REQUIRE(check_str == "-2.24444e1");
  check_str = ft::ryu::to_string(-0.1);
  REQUIRE(check_str == "-1e-1");
  check_str = ft::ryu::to_string(-0.22);
  REQUIRE(check_str == "-2.2e-1");
  check_str = ft::ryu::to_string(-1.22);
  REQUIRE(check_str == "-1.22e0");
  check_str = ft::ryu::to_string(-22.22);
  REQUIRE(check_str == "-2.222e1");
  check_str = ft::ryu::to_string(-std::numeric_limits<double>::min());
  REQUIRE(check_str == "-2.2250738585072013e-308");
  check_str = ft::ryu::to_string(-std::numeric_limits<double>::max());
  REQUIRE(check_str == "-1.7976931348623158e308");
}

TEST_CASE("basic parsenumber", "[ryu]")
{
  ft::parsed_string parsedString;

  std::string small_pi = "3.1415";
  auto result = ft::parseNumber(small_pi.data(), small_pi.size(), parsedString);
  REQUIRE(result == ft::parse_string_error::ok);

  std::string big_number = "1.2345";
  result = ft::parseNumber(big_number.data(), big_number.size(), parsedString);
  REQUIRE(result == ft::parse_string_error::ok);
  
  std::string lessone = "0.3";
  result = ft::parseNumber(lessone.data(), lessone.size(), parsedString);
  REQUIRE(result == ft::parse_string_error::ok);
}

TEST_CASE("basic convert", "[ryu]")
{
  ft::parsed_string parsedString;
  std::string lessone = "0.3";
  auto result = ft::parseNumber(lessone.data(), lessone.size(), parsedString);
  REQUIRE(result == ft::parse_string_error::ok);
  double three = 0.3;
  double d = ft::convertToNumber<double>(parsedString);
}

TEST_CASE("basic roundtrip", "[float tools]")
{
  uint64_t d_value = (uint64_t(1) << 62) + 2;
  for (int i = 0; i < 1000000; i++, d_value++)
  {
    double d;
    memcpy(&d, &d_value, sizeof(d));
    std::string d_str = ft::ryu::to_string(d);
    double d2 = ft::to_double(d_str.data(), d_str.size());
    uint64_t d2_value;
    memcpy(&d2_value, &d2, sizeof(d2));
    if (d != d2)
    {
      fmt::print(stderr, "d {} d_val {} d_str {} d2 {} d2_val {}\n", d, d_value, d_str, d2, d2_value);
      REQUIRE(d == d2);
    }
  }
}

TEST_CASE("parse_convert", "[float tools]")
{
  std::string regular_1 = "123456789";
  double d = ft::to_double(regular_1.data(), regular_1.size());
  uint64_t value;
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 123456789.0);

  std::string regular_2 = "0.0004";
  d = ft::to_double(regular_2.data(), regular_2.size());
  memcpy(&value, &d, sizeof(d));
  double compare = 0.0004;
  uint64_t value_compare;
  memcpy(&value_compare, &compare, sizeof(compare));
  REQUIRE(d == compare);

  std::string regular_3 = "0.0004e10";
  d = ft::to_double(regular_3.data(), regular_3.size());
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 0.0004e10);
  
  std::string regular_4 = "234567.5326e-100";
  d = ft::to_double(regular_4.data(), regular_4.size());
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 234567.5326e-100);
}

TEST_CASE("parse_convert_extremes", "[float tools]")
{
  std::string too_large = "1.7976931348623157e309";
  double d = ft::to_double(too_large.data(), too_large.size());
  uint64_t value;
  memcpy(&value, &d, sizeof(d));
  REQUIRE(std::isinf(d));

  std::string just_large = "1.7976931348623157e308";
  d = ft::to_double(just_large.data(), just_large.size());
  REQUIRE(d == 1.7976931348623157e308);

  std::string too_small = "4.9406564584124654e-325";
  d = ft::to_double(too_small.data(), too_small.size());
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 0.0);
  
  std::string just_small = "4.9406564584124654e-324";
  d = ft::to_double(just_small.data(), just_small.size());
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 4.9406564584124654e-324);
}

TEST_CASE("random_numbers", "[roundtrip]")
{
  uint64_t range = uint64_t(1) << 50;
  uint64_t global_offset = uint64_t(1) << 0;
  uint64_t max_values_pr_thread = uint64_t(1) << 20;

  auto thread_count = std::thread::hardware_concurrency();
  uint64_t range_pr_thread = range / thread_count;
  uint64_t steps = range_pr_thread / std::min(max_values_pr_thread, range_pr_thread);
  steps |= 1;

  auto task = [steps, range_pr_thread](uint64_t range_start, bool print_progress = false)
  {
    uint64_t double_value;
    double double_number;
    std::string double_str;
    double converted_number;
    uint64_t converted_value;
  
    int percentage = -1;
    for (uint64_t i = 0; i < range_pr_thread; i+= steps)
    {
      if (print_progress)
      {
        int new_percentage = int(double(i) / range_pr_thread * 100.0);
        if (new_percentage != percentage)
        {
          percentage = new_percentage;
          fmt::print(stderr, "\r{} % Done.", percentage);
        }
      }
      double_value = range_start + i;// | (uint64_t(1) << 63);
      memcpy(&double_number, &double_value, sizeof(double_number));
      double_str = ft::ryu::to_string(double_number);
      converted_number = ft::to_double(double_str.data(), double_str.size());
      memcpy(&converted_value, &converted_number, sizeof(converted_number));
      REQUIRE(converted_value == double_value);
    }
  };

  if (thread_count > 1)
  {
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < thread_count; i++)
    {
      auto t_task = [global_offset, i, range_pr_thread, task]()
      {
        uint64_t range_start = global_offset + (i * range_pr_thread);
        task(range_start, i == 0);
      };
      threads.emplace_back(t_task);
    }

    for (auto& thread : threads)
    {
      thread.join();
    }

  }
  else
  {
    task(0, true);
  }
}

TEST_CASE("problematic_values", "[roundtrip]")
{
  uint64_t double_value;
  double double_number;
  std::string double_str;
  double converted_number;
  uint64_t converted_value;
  double stdd;

  double_value = 1500370978;
  memcpy(&double_number, &double_value, sizeof(double_number));
  double_str = ft::ryu::to_string(double_number);
  stdd = strtod(double_str.c_str(), nullptr);
  converted_number = ft::to_double(double_str.data(), double_str.size());
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  REQUIRE(converted_number == stdd);
  REQUIRE(converted_value == double_value);

  double_value = 4611686019697841006;
  memcpy(&double_number, &double_value, sizeof(double_number));
  double_str = ft::ryu::to_string(double_number);
  converted_number = ft::to_double(double_str.data(), double_str.size());
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  stdd = strtod(double_str.c_str(), nullptr);
  REQUIRE(converted_value == double_value);
  REQUIRE(converted_number == stdd);

  double_number= -1.1234e-10;
  memcpy(&double_value, &double_number, sizeof(double_value));
  double_str = ft::ryu::to_string(double_number);
  converted_number = ft::to_double(double_str.data(), double_str.size());
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  stdd = strtod(double_str.c_str(), nullptr);
  REQUIRE(converted_value == double_value);
  REQUIRE(converted_number == stdd);
  
  double_value = 1297037298273091867;
  memcpy(&double_number, &double_value, sizeof(double_number));
  double_str = ft::ryu::to_string(double_number);
  converted_number = ft::to_double(double_str.data(), double_str.size());
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  stdd = strtod(double_str.c_str(), nullptr);
  REQUIRE(converted_value == double_value);
  REQUIRE(converted_number == stdd);
}

TEST_CASE("basic_str_to_float", "[float tests]")
{
  float float_number = 0.12345f;
  std::string float_str = ft::ryu::to_string(float_number);
  float test = ft::to_float(float_str.c_str(), float_str.size());
  float compare = 0.12345f;
  REQUIRE(test == compare);
}

TEST_CASE("roundtrip_all_floats", "[float tests]")
{
  auto thread_count = std::thread::hardware_concurrency();
  uint64_t range_pr_thread = (uint64_t(0x7f7fffff) + 1) / thread_count;

  {
    auto task = [range_pr_thread](uint64_t range_start, bool print_progress = false)
    {
      float float_number;
      std::string float_str;
      uint32_t converted_value;
      float converted_number;
      int percentage = -1;
      uint32_t range_end = range_start + uint32_t(range_pr_thread);
      for (uint32_t float_value = range_start; float_value < range_end; float_value++)
      {
        if (print_progress)
        {
          int new_percentage = int(double(float_value) / range_pr_thread * 100.0);
          if (new_percentage != percentage)
          {
            percentage = new_percentage;
            fmt::print(stderr, "\r{} % Negative Done.", percentage);
          }
        }
        uint32_t test_value = float_value | uint32_t(1) << 31;
        memcpy(&float_number, &test_value, sizeof(float_number));
        float_str = ft::ryu::to_string(float_number);
        converted_number = ft::to_float(float_str.c_str(), float_str.size());
        memcpy(&converted_value, &converted_number, sizeof(converted_value));
        REQUIRE(test_value == converted_value);
      }
    };

    if (thread_count > 1)
    {
      std::vector<std::thread> threads;
      for (uint32_t i = 0; i < thread_count; i++)
      {
        auto t_task = [range_pr_thread, task](uint32_t i)
        {
          uint64_t range_start = i * range_pr_thread;
          task(range_start, i == 0);
        };
        threads.emplace_back(t_task, i);
      }

      for (auto& thread : threads)
      {
        thread.join();
      }
    }
    else
    {
      task(0, true);
    }
  }
  
  fmt::print(stderr, "\n");

  auto task = [range_pr_thread](uint64_t range_start, bool print_progress = false)
  {
    float float_number;
    std::string float_str;
    uint32_t converted_value;
    float converted_number;
    int percentage = -1;
    uint32_t range_end = range_start + uint32_t(range_pr_thread);
    for (uint32_t float_value = range_start; float_value < range_end; float_value++)
    {
      if (print_progress)
      {
        int new_percentage = int(double(float_value) / range_pr_thread * 100.0);
        if (new_percentage != percentage)
        {
          percentage = new_percentage;
          fmt::print(stderr, "\r{} % Positive Done.", percentage);
        }
      }
      memcpy(&float_number, &float_value, sizeof(float_number));
      float_str = ft::ryu::to_string(float_number);
      converted_number = ft::to_float(float_str.c_str(), float_str.size());
      memcpy(&converted_value, &converted_number, sizeof(converted_value));
      REQUIRE(float_value == converted_value);
    }
  };

  if (thread_count > 1)
  {
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < thread_count; i++)
    {
      auto t_task = [range_pr_thread, task](uint32_t i)
      {
        uint64_t range_start = i * range_pr_thread;
        task(range_start, i == 0);
      };
      threads.emplace_back(t_task, i);
    }

    for (auto& thread : threads)
    {
      thread.join();
    }
  }
  else
  {
    task(0, true);
  }
  fmt::print(stderr, "\n");
}

static void assert_float(uint32_t float_value)
{
  float float_number;
  std::string float_str;
  float converted_number;
  uint32_t converted_value;

  memcpy(&float_number, &float_value, sizeof(float_number));
  float_str = ft::ryu::to_string(float_number);
  converted_number = ft::to_float(float_str.data(), float_str.size());
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  REQUIRE(float_value == converted_value);
}

TEST_CASE("problematic_float_values", "[float tests]")
{
  assert_float(uint32_t(2130706433));
  assert_float(uint32_t(1325400268));
  assert_float(uint32_t(1317011660));
  assert_float(uint32_t(1336934627));
  assert_float(uint32_t(1300234244));
  assert_float(uint32_t(1470750605));
  assert_float(uint32_t(1291845636));
  assert_float(uint32_t(1283457030));
  assert_float(uint32_t(1283457028));
  assert_float(uint32_t(1283457054));
  assert_float(uint32_t(1275069440));
  assert_float(uint32_t(1283457025));
  assert_float(uint32_t(1275068680));
  assert_float(uint32_t(1275068550));
  assert_float(uint32_t(1275068530));
  assert_float(uint32_t(1342177391));
  assert_float(uint32_t(1275068520));
  assert_float(uint32_t(1275068490));
  assert_float(uint32_t(1275068440));
  assert_float(uint32_t(134217734));
  assert_float(uint32_t(1336934502));
  assert_float(uint32_t(1275068480));
  assert_float(uint32_t(1275068460));
  assert_float(uint32_t(1275068430));
  assert_float(uint32_t(1275068420));
  assert_float(uint32_t(1275068425));
  assert_float(uint32_t(1258291200));
  assert_float(uint32_t(7778));
  assert_float(uint32_t(5160));
  assert_float(uint32_t(4194304));
  assert_float(uint32_t(4194305));
  assert_float(uint32_t(671088640));
  assert_float(uint32_t(1336934636));
  assert_float(uint32_t(1336934694));
  assert_float(uint32_t(402653184));
  assert_float(uint32_t(1336934436));
  assert_float(uint32_t(5160));
  assert_float(uint32_t(79));
}
