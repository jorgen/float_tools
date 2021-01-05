#include <float_tools.h>

#include "catch.hpp"

#include <stdint.h>

#include <stdio.h>
#include <limits>

#include <random>


#include <thread>
#include <chrono>

#include <fmt/printf.h>

TEST_CASE("regular_doubles", "[ryu]")
{
  std::string check_str = ft::ryu::to_string(1.0);
  REQUIRE(check_str == "1.0");
  check_str = ft::ryu::to_string(1.1);
  REQUIRE(check_str == "1.1");
  check_str = ft::ryu::to_string(22.1);
  REQUIRE(check_str == "22.1");
  check_str = ft::ryu::to_string(333.1);
  REQUIRE(check_str == "333.1");
  check_str = ft::ryu::to_string(22.4444);
  REQUIRE(check_str == "22.4444");
  check_str = ft::ryu::to_string(0.1);
  REQUIRE(check_str == "0.1");
  check_str = ft::ryu::to_string(0.22);
  REQUIRE(check_str == "0.22");
  check_str = ft::ryu::to_string(1.22);
  REQUIRE(check_str == "1.22");
  check_str = ft::ryu::to_string(22.22);
  REQUIRE(check_str == "22.22");
  check_str = ft::ryu::to_string(std::numeric_limits<double>::min());
  REQUIRE(check_str == "2.2250738585072014e-308");
  check_str = ft::ryu::to_string(std::numeric_limits<double>::max());
  REQUIRE(check_str == "1.7976931348623157e308");
  check_str = ft::ryu::to_string(0.0);
  REQUIRE(check_str == "0.0");

}

TEST_CASE("small test negative", "[ryu]")
{
  std::string check_str = ft::ryu::to_string(-1.0);
  REQUIRE(check_str == "-1.0");
  check_str = ft::ryu::to_string(-1.1);
  REQUIRE(check_str == "-1.1");
  check_str = ft::ryu::to_string(-22.1);
  REQUIRE(check_str == "-22.1");
  check_str = ft::ryu::to_string(-333.1);
  REQUIRE(check_str == "-333.1");
  check_str = ft::ryu::to_string(-22.4444);
  REQUIRE(check_str == "-22.4444");
  check_str = ft::ryu::to_string(-0.1);
  REQUIRE(check_str == "-0.1");
  check_str = ft::ryu::to_string(-0.22);
  REQUIRE(check_str == "-0.22");
  check_str = ft::ryu::to_string(-1.22);
  REQUIRE(check_str == "-1.22");
  check_str = ft::ryu::to_string(-22.22);
  REQUIRE(check_str == "-22.22");
  check_str = ft::ryu::to_string(-std::numeric_limits<double>::min());
  REQUIRE(check_str == "-2.2250738585072014e-308");
  check_str = ft::ryu::to_string(-std::numeric_limits<double>::max());
  REQUIRE(check_str == "-1.7976931348623157e308");
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
  REQUIRE(d == three);
}

TEST_CASE("zero_convert", "[float tools]")
{

  const char* end_ptr;
  std::string zero = "0";
  double d;
  auto result = ft::to_double(zero.data(), zero.size(), d, end_ptr);
  uint64_t test;
  memcpy(&test, &d, sizeof(test));
  REQUIRE(result == ft::parse_string_error::ok);
  REQUIRE(test == uint64_t(0));

  zero = "-0";
  result = ft::to_double(zero.data(), zero.size(), d, end_ptr);
  memcpy(&test, &d, sizeof(test));

  REQUIRE(result == ft::parse_string_error::ok);
  REQUIRE(test == (uint64_t(1)<<63));
  
  zero = "0.0";
  result = ft::to_double(zero.data(), zero.size(), d, end_ptr);
  memcpy(&test, &d, sizeof(test));

  REQUIRE(result == ft::parse_string_error::ok);
  REQUIRE(test == (uint64_t(0)));

}

TEST_CASE("parse_convert", "[float tools]")
{
  std::string regular_1 = "123456789";
  const char* end_ptr;
  double d;
  auto result = ft::to_double(regular_1.data(), regular_1.size(), d, end_ptr);
  uint64_t value;
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 123456789.0);

  std::string regular_2 = "0.0004";
  result = ft::to_double(regular_2.data(), regular_2.size(), d, end_ptr);
  memcpy(&value, &d, sizeof(d));
  double compare = 0.0004;
  uint64_t value_compare;
  memcpy(&value_compare, &compare, sizeof(compare));
  REQUIRE(d == compare);

  std::string regular_3 = "0.0004e10";
  result = ft::to_double(regular_3.data(), regular_3.size(), d, end_ptr);
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 0.0004e10);
  
  std::string regular_4 = "234567.5326e-100";
  result = ft::to_double(regular_4.data(), regular_4.size(), d, end_ptr);
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 234567.5326e-100);
}

TEST_CASE("parse_convert_extremes", "[float tools]")
{
  std::string too_large = "1.7976931348623157e309";
  const char* end_ptr;
  double d;
  auto result = ft::to_double(too_large.data(), too_large.size(), d, end_ptr);
  uint64_t value;
  memcpy(&value, &d, sizeof(d));
  REQUIRE(ft::is_inf(d));

  std::string just_large = "1.7976931348623157e308";
  result = ft::to_double(just_large.data(), just_large.size(), d, end_ptr);
  REQUIRE(d == 1.7976931348623157e308);

  std::string too_small = "4.9406564584124654e-325";
  result = ft::to_double(too_small.data(), too_small.size(), d, end_ptr);
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 0.0);
  
  std::string just_small = "4.9406564584124654e-324";
  result = ft::to_double(just_small.data(), just_small.size(), d, end_ptr);
  memcpy(&value, &d, sizeof(d));
  REQUIRE(d == 4.9406564584124654e-324);
}

TEST_CASE("random_numbers", "[roundtrip]")
{
  uint64_t range = UINT64_C(0x7FEFFFFFFFFFFFFF) - 1;
  uint64_t max_values_pr_thread = uint64_t(1) << 15;
  int random_step_values = 509;
  auto thread_count = std::thread::hardware_concurrency();
  uint64_t range_pr_thread = range / thread_count;

  auto task = [range_pr_thread, max_values_pr_thread](uint64_t range_start, const std::vector<int> &offsets, bool negative, bool print_progress = false)
  {
    uint64_t double_value;
    double double_number;
    std::string double_str;
    double converted_number;
    uint64_t converted_value;
    const char* end_ptr;

    auto step_size_in_thread = range_pr_thread / max_values_pr_thread;
    auto scale_to_get_avrage_step = step_size_in_thread / std::max(offsets.size() / 2, size_t(1));

    int percentage = -1;
    uint64_t i = range_start;
    uint64_t range_end = range_start + range_pr_thread;
    uint64_t step_index = 0;
    while (i < range_end)
    {
      if (print_progress)
      {
        int new_percentage = int(double(i) / double(range_pr_thread) * 100.0);
        if (new_percentage != percentage)
        {
          percentage = new_percentage;
          if (negative)
            fmt::print(stderr, "\r{} % Doubles Done Negative.", percentage);
          else
            fmt::print(stderr, "\r{} % Doubles Done Positive.", percentage);
        }
      }
      double_value = i; 
      if (negative)
        double_value |= (uint64_t(1) << 63);
      memcpy(&double_number, &double_value, sizeof(double_number));
      double_str = ft::ryu::to_string(double_number);
      auto result = ft::to_double(double_str.data(), double_str.size(), converted_number, end_ptr);
      REQUIRE(result == ft::parse_string_error::ok);
      memcpy(&converted_value, &converted_number, sizeof(converted_number));
      REQUIRE(converted_value == double_value);

      step_index++;
      i += uint64_t(offsets[step_index % offsets.size()]) * scale_to_get_avrage_step;
    }
    //fmt::print(stderr, "Thread ran for {} iterations\n", step_index);
  };

  std::vector<std::thread> threads;
  for (uint32_t i = 0; i < thread_count; i++)
  {
    auto t_task = [range_pr_thread, task, random_step_values](int local_i)
    {
      std::vector<int> offsets;
      offsets.reserve(size_t(random_step_values));
      for (int s = 0; s < random_step_values; s++)
      {
        offsets.push_back(s);
      }
      std::random_device rd;
      std::mt19937 g(rd() + std::random_device::result_type(local_i));
      std::shuffle(offsets.begin(), offsets.end(), g);
      
      uint64_t range_start = uint64_t(local_i) * range_pr_thread;
      task(range_start, offsets, false, local_i == 0);
    };
    threads.emplace_back(t_task, i);
  }

  for (auto& thread : threads)
  {
    thread.join();
  }
  threads.clear();

  for (uint32_t i = 0; i < thread_count; i++)
  {
    auto t_task = [range_pr_thread, task, random_step_values](int local_i)
    {
      std::vector<int> offsets;
      offsets.reserve(size_t(random_step_values));
      for (int s = 0; s < random_step_values; s++)
      {
        offsets.push_back(s);
      }
      std::random_device rd;
      std::mt19937 g(rd() + std::random_device::result_type(local_i));
      std::shuffle(offsets.begin(), offsets.end(), g);
      
      uint64_t range_start = uint64_t(local_i) * range_pr_thread;
      task(range_start, offsets, true, local_i == 0);
    };
    threads.emplace_back(t_task, i);
  }

  for (auto& thread : threads)
  {
    thread.join();
  }
}

TEST_CASE("random_numbers_float", "[roundtrip]")
{
  uint32_t range = uint32_t(0x7f7fffff) - 1;
  uint32_t max_values_pr_thread = uint32_t(1) << 15;
  int random_step_values = 509;
  auto thread_count = std::thread::hardware_concurrency();
  uint32_t range_pr_thread = range / thread_count;

  auto task = [range_pr_thread, max_values_pr_thread](uint32_t range_start, const std::vector<uint32_t> &offsets, bool negative, bool print_progress = false)
  {
    uint32_t float_value;
    float float_number;
    std::string float_str;
    float converted_number;
    uint32_t converted_value;
    const char* end_ptr;

    auto step_size_in_thread = range_pr_thread / max_values_pr_thread;
    auto scale_to_get_avrage_step = step_size_in_thread / std::max(offsets.size() / 2, size_t(1));

    int percentage = -1;
    uint32_t i = range_start;
    uint32_t range_end = range_start + range_pr_thread;
    uint32_t step_index = 0;
    while (i < range_end)
    {
      if (print_progress)
      {
        int new_percentage = int(float(i) / float(range_pr_thread) * 100.0f);
        if (new_percentage != percentage)
        {
          percentage = new_percentage;
          if (negative)
            fmt::print(stderr, "\r{} % Floats Done Negative.", percentage);
          else
            fmt::print(stderr, "\r{} % Floats Done Positive.", percentage);
        }
      }
      float_value = i; 
      if (negative)
        float_value |= (uint32_t(1) << 31);
      memcpy(&float_number, &float_value, sizeof(float_number));
      float_str = ft::ryu::to_string(float_number);
      auto result = ft::to_float(float_str.data(), float_str.size(), converted_number, end_ptr);
      REQUIRE(result == ft::parse_string_error::ok);
      memcpy(&converted_value, &converted_number, sizeof(converted_number));
      REQUIRE(converted_value == float_value);

      step_index++;
      i += offsets[step_index % offsets.size()] * scale_to_get_avrage_step;
    }
    //fmt::print(stderr, "Thread ran for {} iterations\n", step_index);
  };

  std::vector<std::thread> threads;
  for (uint32_t i = 0; i < thread_count; i++)
  {
    auto t_task = [range_pr_thread, task, random_step_values](int local_i)
    {
      std::vector<uint32_t> offsets;
      offsets.reserve(size_t(random_step_values));
      for (int s = 0; s < random_step_values; s++)
      {
        offsets.push_back(uint32_t(s));
      }
      std::random_device rd;
      std::mt19937 g(rd() + std::random_device::result_type(local_i));
      std::shuffle(offsets.begin(), offsets.end(), g);
      
      uint32_t range_start = uint32_t(local_i) * range_pr_thread;
      task(range_start, offsets, false, local_i == 0);
    };
    threads.emplace_back(t_task, i);
  }

  for (auto& thread : threads)
  {
    thread.join();
  }
  threads.clear();

  for (uint32_t i = 0; i < thread_count; i++)
  {
    auto t_task = [range_pr_thread, task, random_step_values](int local_i)
    {
      std::vector<uint32_t> offsets;
      offsets.reserve(size_t(random_step_values));
      for (int s = 0; s < int(random_step_values); s++)
      {
        offsets.push_back(uint32_t(s));
      }
      std::random_device rd;
      std::mt19937 g(rd() + std::random_device::result_type(local_i));
      std::shuffle(offsets.begin(), offsets.end(), g);
      
      uint32_t range_start = uint32_t(local_i) * range_pr_thread;
      task(range_start, offsets, true, local_i == 0);
    };
    threads.emplace_back(t_task, i);
  }

  for (auto& thread : threads)
  {
    thread.join();
  }
}

static void assert_double(uint64_t double_value)
{
  double double_number;
  std::string double_str;
  double converted_number;
  uint64_t converted_value;
  std::string charconv_str;
  charconv_str.resize(30);
  std::string ryu_str;
  ryu_str.resize(30);
  const char* endptr;

  memcpy(&double_number, &double_value, sizeof(double_number));
  double_str = ft::ryu::to_string(double_number);
  //f2s_buffered(double_number, ryu_str.data());
  auto result = ft::to_double(double_str.data(), double_str.size(), converted_number, endptr);
  REQUIRE(result == ft::parse_string_error::ok);
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  REQUIRE(double_value == converted_value);
}

TEST_CASE("problematic_values", "[roundtrip]")
{
  //assert_double(UINT64_C(9218868437280221096)); //write test that verifies nan and inf
  assert_double(UINT64_C(18453984117258189));
  assert_double(UINT64_C(1500370978));
  assert_double(UINT64_C(4611686019697841006));
  assert_double(1297037298273091867);
  assert_double(UINT64_C(4812201603180978432));
  double some_value_number = -1.1234e-10;
  uint64_t some_value;
  memcpy(&some_value, &some_value_number, sizeof(some_value));
  assert_double(some_value);
  
}

TEST_CASE("basic_str_to_float", "[float tests]")
{
  float float_number = 0.12345f;
  std::string float_str = ft::ryu::to_string(float_number);
  float test;
  const char* endptr;
  auto result = ft::to_float(float_str.c_str(), float_str.size(), test, endptr);
  REQUIRE(result == ft::parse_string_error::ok);
  float compare = 0.12345f;
  REQUIRE(test == compare);
}

TEST_CASE("roundtrip_all_floats", "[float tests]")
{
#if 0  
  auto start = std::chrono::system_clock::now();

  auto thread_count = std::thread::hardware_concurrency();
  uint32_t range_pr_thread = (uint32_t(0x7f7fffff) + 1) / thread_count;

  {
    auto task = [range_pr_thread](uint32_t range_start, bool print_progress = false)
    {
      float float_number;
      std::string float_str;
      uint32_t converted_value;
      float converted_number;
      const char* endptr;
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
        auto result = ft::to_float(float_str.c_str(), float_str.size(), converted_number, endptr);
        REQUIRE(result == ft::parse_string_error::ok);
        memcpy(&converted_value, &converted_number, sizeof(converted_value));
        REQUIRE(test_value == converted_value);
      }
    };

    if (thread_count > 1)
    {
      std::vector<std::thread> threads;
      for (uint32_t i = 0; i < thread_count; i++)
      {
        auto t_task = [range_pr_thread, task](uint32_t local_i)
        {
          uint32_t range_start = local_i * range_pr_thread;
          task(range_start, local_i == 0);
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

  auto task = [range_pr_thread](uint32_t range_start, bool print_progress = false)
  {
    float float_number;
    std::string float_str;
    uint32_t converted_value;
    float converted_number;
    const char* endptr;
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
      auto result = ft::to_float(float_str.c_str(), float_str.size(), converted_number, endptr);
      REQUIRE(result == ft::parse_string_error::ok);
      memcpy(&converted_value, &converted_number, sizeof(converted_value));
      REQUIRE(float_value == converted_value);
    }
  };

  if (thread_count > 1)
  {
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < thread_count; i++)
    {
      auto t_task = [range_pr_thread, task](uint32_t local_i)
      {
        uint32_t range_start = local_i * range_pr_thread;
        task(range_start, local_i == 0);
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
  auto end = std::chrono::system_clock::now();
  auto elapsed = end - start;
  std::chrono::seconds seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
  fmt::print(stderr, "Time for roundtripping all floats is: {} seconds.\n", seconds_elapsed.count());
#endif  
}

static void assert_float(uint32_t float_value)
{
  float float_number;
  std::string float_str;
  float converted_number;
  uint32_t converted_value;
  std::string charconv_str;
  charconv_str.resize(30);
  std::string ryu_str;
  ryu_str.resize(30);
  const char* endptr;

  memcpy(&float_number, &float_value, sizeof(float_number));
  float_str = ft::ryu::to_string(float_number);
  auto result = ft::to_float(float_str.data(), float_str.size(), converted_number, endptr);
  REQUIRE(result == ft::parse_string_error::ok);
  memcpy(&converted_value, &converted_number, sizeof(converted_number));
  REQUIRE(float_value == converted_value);
}

TEST_CASE("problematic_float_values", "[float tests]")
{
  assert_float(uint32_t(3618111833));
  assert_float(uint32_t(1336934627));
  assert_float(uint32_t(1325400268));
  assert_float(uint32_t(2130706433));
  assert_float(uint32_t(1317011660));
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

#define ASSERT_FLOAT_STR(TYPE, X) do { \
  std::string str = #X; \
  TYPE converted_float; \
  const char* endptr; \
  auto result = ft::to_ieee_t(str.data(), str.size(), converted_float, endptr); \
  REQUIRE(result == ft::parse_string_error::ok); \
  REQUIRE(converted_float == TYPE(X)); \
  } while (0) \

TEST_CASE("problematic_strings", "string_to_float")
{
  ASSERT_FLOAT_STR(float, 32587.403333333333333333333);
  ASSERT_FLOAT_STR(double, 32587.403333333333333333333);
  ASSERT_FLOAT_STR(float, 99999.999999999999999999999);
  ASSERT_FLOAT_STR(double, 99999.999999999999999999999);

}
