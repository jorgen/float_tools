/************************************************************************
* This is free and unencumbered software released into the public domain.
* 
* Anyone is free to copy, modify, publish, use, compile, sell, or
* distribute this software, either in source code form or as a compiled
* binary, for any purpose, commercial or non-commercial, and by any
* means.
* 
* In jurisdictions that recognize copyright laws, the author or authors
* of this software dedicate any and all copyright interest in the
* software to the public domain. We make this dedication for the benefit
* of the public at large and to the detriment of our heirs and
* successors. We intend this dedication to be an overt act of
* relinquishment in perpetuity of all present and future rights to this
* software under copyright law.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
* For more information, please refer to <http://unlicense.org/>
************************************************************************/

#ifndef FLOAT_TOOLS_H
#define FLOAT_TOOLS_H

#include <stdint.h>
#include <math.h>
#include <ttmath/ttmath.h>
#include <fmt/printf.h>
#include <algorithm>
#include <string.h>
#include <array>

namespace ft
{
  struct float_base10
  {
    uint8_t negative;
    uint8_t inf;
    uint8_t nan;
    uint8_t significand_digit_count;
    int32_t exp;
    uint64_t significand;
  };

  struct parsed_string : float_base10
  {
    const char* endptr;
  };

  enum class parse_string_error
  {
    ok,
    invalid_format,
    multiple_commas,
    empty_string,
    illegal_exponent_value
  };

  constexpr static inline uint64_t high(uint64_t x) { return x >> 32; }
  constexpr static inline uint64_t low(uint64_t x) { return x & ~uint32_t(0); }

  template<int shift = 1>
  inline void left_shift(uint64_t(&a)[2])
  {
    a[1] = a[1] << shift | (a[0] >> ((sizeof(uint64_t) * 8) - shift));
    a[0] = a[0] << shift;
  }

  template<int shift = 1>
  inline void left_shift(uint64_t&a)
  {
    a = a << shift;
  }

  inline void right_shift(uint64_t(&a)[2])
  {
    a[0] = a[0] >> 1 | (a[1] << ((sizeof(uint64_t) * 8) - 1));
    a[1] = a[1] >> 1;
  }

  inline void right_shift(uint64_t&a)
  {
    a = a >> 1;
  }

  inline uint64_t mask32(uint64_t a)
  {
    return a & ((uint64_t(1) << 32) - 1);
  }

  inline void add(const uint64_t(&a)[2], uint64_t(&b)[2])
  {
    uint64_t tmplow[2];
    uint64_t tmphigh[2];
    tmplow[0] = low(a[0]) + low(b[0]);
    tmplow[1] = low(a[1]) + low(b[1]);
    tmphigh[0] = high(a[0]) + high(b[0]);
    tmphigh[1] = high(a[1]) + high(b[1]);

    tmphigh[0] += tmplow[0] >> 32;
    tmplow[1] += tmphigh[0] >> 32;
    tmphigh[1] += tmplow[1] >> 32;

    b[0] = mask32(tmplow[0]) | (tmphigh[0] << 32);
    b[1] = mask32(tmplow[1]) | (tmphigh[1] << 32);
  }

  inline void add(const uint64_t&a, uint64_t&b)
  {
    b += a;
  }

  inline void divide_by_10(uint64_t(&a)[2])
  {
    uint64_t remainder = a[1] % 10;
    a[1] /= 10;
    uint64_t high_pluss_reminder = high(a[0]) + (remainder << 32);
    uint64_t high_d = high_pluss_reminder / 10;
    uint64_t high_r = high_pluss_reminder % 10;
    uint64_t low_d = (low(a[0]) + (high_r << 32)) / 10;
    a[0] = high_d << 32 | low_d;
  }

  inline void divide_by_10(uint64_t&a)
  {
    a /= 10;
  }

  template<typename T>
  struct float_info 
  {
  };

  template<>
  struct float_info<double>
  {
    static inline constexpr int mentissa_width() noexcept { return 52; }
    static inline constexpr int exponent_width() noexcept { return 11; }
    static inline constexpr int bias() noexcept { return (1 << (exponent_width() - 1)) - 1; }
    static inline constexpr int max_base10_exponent() noexcept { return 308; }
    static inline constexpr int min_base10_exponent() noexcept { return -324; }

    using str_to_float_conversion_type = uint64_t[2];
    static inline constexpr int str_to_float_binary_exponen_init() noexcept { return  64 + 60; }
    static inline constexpr uint64_t str_to_float_mask() noexcept { return  ~((uint64_t(1) << 60) - 1); }
    static inline constexpr uint64_t str_to_float_top_bit_in_mask() noexcept { return str_to_float_mask() << 3; }
    static inline constexpr bool conversion_type_has_mask(const str_to_float_conversion_type& a) noexcept { return a[1] & str_to_float_mask(); }
    static inline constexpr bool conversion_type_has_top_bit_in_mask(const str_to_float_conversion_type& a) noexcept { return a[1] & str_to_float_top_bit_in_mask(); }
    static inline constexpr bool conversion_type_is_null(const str_to_float_conversion_type& a) noexcept { return !a[0] && !a[1]; }
    static inline void copy_denormal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, double& to_digit)
    {
      uint64_t q = a[1];
      q += 512;
      q >>= 9;
      int expo_shift = -binary_exponent;
      uint64_t round = (uint64_t(1) << (expo_shift - 1)) - 1;
      q += round;
      q >>= expo_shift;
      if (negative)
        q |= uint64_t(1) << 63;
      memcpy(&to_digit, &q, sizeof(q));
    }
    static inline void copy_normal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, double& to_digit)
    {
      uint64_t q = a[1] & ~str_to_float_mask();
      q += 128;
      q >>= 8;
      q |= uint64_t(binary_exponent) << mentissa_width();
      if (negative)
        q |= uint64_t(1) << 63;
      memcpy(&to_digit, &q, sizeof(q));
    }
  };
  inline void assign_significand_to_float_conversion_type(const float_base10 &significand, uint64_t(&a)[2])
  {
    a[0] = significand.significand;
    a[1] = 0;
  }

  inline void copy_conversion_type(const uint64_t(&a)[2], uint64_t(&b)[2])
  {
    memcpy(&b, &a, sizeof(b));
  }
  
  template<>
  struct float_info<float>
  {
    static inline constexpr int mentissa_width() noexcept { return 23; }
    static inline constexpr int exponent_width() noexcept { return 8; }
    static inline constexpr int bias() noexcept { return (1 << (exponent_width() - 1)) - 1; }
    static inline constexpr int max_base10_exponent() noexcept { return 38; }
    static inline constexpr int min_base10_exponent() noexcept { return -45; }

    using str_to_float_conversion_type = uint64_t;
    static inline constexpr int str_to_float_binary_exponen_init() noexcept { return 60; }
    static inline constexpr uint64_t str_to_float_mask() noexcept { return  ~((uint64_t(1) << 60) - 1); }
    static inline constexpr uint64_t str_to_float_top_bit_in_mask() noexcept { return str_to_float_mask() << 3; }
    static inline constexpr bool conversion_type_has_mask(const str_to_float_conversion_type& a) noexcept { return a & str_to_float_mask(); }
    static inline constexpr bool conversion_type_has_top_bit_in_mask(const str_to_float_conversion_type& a) noexcept { return a & str_to_float_top_bit_in_mask(); }
    static inline constexpr bool conversion_type_is_null(const str_to_float_conversion_type& a) noexcept { return !a; }
    static inline void copy_denormal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, float &to_digit)
    {
      uint64_t q = a;
      q += uint64_t(1) << (38 - 1);
      q >>= 38;
      int expo_shift = -binary_exponent;
      uint64_t round = (uint64_t(1) << (expo_shift - 1)) - 1;
      q += round;
      q >>= expo_shift;
      if (negative)
        q |= uint64_t(1) << 31;
      uint32_t to_copy = uint32_t(q);
      memcpy(&to_digit, &to_copy, sizeof(to_copy));
    }
    static inline void copy_normal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, float &to_digit)
    {
      uint64_t q = a & ~str_to_float_mask();
      q += uint64_t(1) << (37 - 1);
      q >>= 37;
      q |= uint64_t(binary_exponent) << mentissa_width();
      if (negative)
        q |= uint64_t(1) << 31;
      uint32_t to_copy = uint32_t(q);
      memcpy(&to_digit, &to_copy, sizeof(to_copy));
    }
  };

  inline void assign_significand_to_float_conversion_type(const float_base10 &significand, uint64_t &a)
  {
    a = significand.significand;
  }

  inline void copy_conversion_type(const uint64_t &a, uint64_t&b)
  {
    b = a;
  }

  namespace ryu
  {
#include "cache_ryu.h" //include cache table in namespace

    constexpr static const double log_10_2 = 0.30102999566398114;
    constexpr static const double log_10_5 = 0.6989700043360189;
    constexpr static const double log_2_5 = 2.321928094887362;

    constexpr static const int max_double_5_pow_q = 23; //floor(log_5(1 << (mentissawidth + 2)))
    constexpr static const int max_double_2_pow_q = 54; //floor(log_2(1 << (mentissawidth + 2)))

    template<typename T>
    inline void get_parts(T f, bool& negative, int32_t& exp, uint64_t& mentissa)
    {
      uint64_t bits = 0;
      static_assert(sizeof(bits) >= sizeof(f), "Incompatible size");
      memcpy(&bits, &f, sizeof(f));
      exp = int32_t((bits >> float_info<T>::mentissa_width()) & (((uint64_t(1) << float_info<T>::exponent_width()) - 1)));
      mentissa = bits & ((uint64_t(1) << float_info<T>::mentissa_width()) - 1);
      negative = bits >> ((sizeof(f) * 8) - 1);
    }

    template<typename T>
    inline void normalize(int& exp, uint64_t& mentissa)
    {
      if (exp)
      {
        mentissa += uint64_t(1) << float_info<T>::mentissa_width();
        exp = exp - float_info<T>::bias() - float_info<T>::mentissa_width();
      }
      else
      {
        exp = 1 - float_info<T>::bias() - float_info<T>::mentissa_width();
      }
    }

    static void compute_shortest(uint64_t a, uint64_t b, uint64_t c, bool accept_smaller, bool accept_larger, bool break_tie_down, int& exponent_adjuster, uint64_t& shortest_base10)
    {
      int i = 0;
      if (!accept_larger)
        c -= 1;
      bool all_a_zero = true;
      bool all_b_zero = true;
      uint64_t a_next = a / 10;
      uint32_t a_remainder = a % 10;
      uint64_t b_next = b / 10;
      uint32_t b_remainder = b % 10;
      uint64_t c_next = c / 10;
      uint32_t c_remainder = c % 10;
      while (a_next < c_next)
      {
        all_b_zero &= bool(!b_remainder);
        all_a_zero &= bool(!a_remainder);
        a = a_next;
        b = b_next;
        c = c_next;
        a_next = a / 10;
        a_remainder = a % 10;
        b_next = b / 10;
        b_remainder = b % 10;
        c_next = c / 10;
        c_remainder = c % 10;
        i++;
      }
      if (accept_smaller && all_a_zero)
      {
        while (!a_remainder)
        {
          all_b_zero &= bool(!b_remainder);

          a_next = a;
          b_next = b;
          c_next = c;

          a = a_next / 10;
          a_remainder = a_next % 10;
          b = b_next / 10;
          b_remainder = b_next % 10;
          c = c_next / 10;
          c_remainder = c_next % 10;
          i++;
        }
      }
      exponent_adjuster = i;

      bool is_tie = b_remainder == 5 && all_b_zero;
      bool want_to_round_down = b_remainder < 5 || (is_tie && break_tie_down);
      bool round_down = (want_to_round_down && (a != b || all_a_zero)) || (b + 1 > c);
      if (round_down)
      {
        shortest_base10 = b;
      }
      else
      {
        shortest_base10 = b + 1;
      }
    }

    template<uint64_t NUMBER, int INDEX, int START_INDEX>
    struct NumberLength
    {
      static int countCharactersInNumber(uint64_t n)
      {
        uint64_t bigN = NUMBER;
        if (n < NUMBER)
          return START_INDEX - INDEX + 1;
        return NumberLength<NUMBER * 10, INDEX - 1, START_INDEX>::countCharactersInNumber(n);
      }

    };
    template<uint64_t NUMBER, int START_INDEX>
    struct NumberLength<NUMBER, 0, START_INDEX>
    {
      static int countCharactersInNumber(uint64_t n)
      {
        return int(START_INDEX);
      }
    };

    int digitsInNumber(uint64_t n)
    {
      return NumberLength<10, 17, 17>::countCharactersInNumber(n);
    }

    template<typename T>
    inline uint64_t multiply_and_shift(uint64_t a, const uint64_t* b, int shift_right)
    {
    }
    template<>
    inline uint64_t multiply_and_shift<double>(uint64_t a, const uint64_t *b, int shift_right)
    {
      uint64_t a0, a1, b0, b1, b2, b3, a0b0, a0b1, a0b2, a0b3, a1b0, a1b1, a1b2, a1b3;
      a0 = low(a); a1 = high(a); b0 = low(b[0]); b1 = high(b[0]); b2 = low(b[1]); b3 = high(b[1]);

      a0b0 = a0 * b0; a0b1 = a0 * b1; a0b2 = a0 * b2; a0b3 = a0 * b3; a1b0 = a1 * b0; a1b1 = a1 * b1; a1b2 = a1 * b2; a1b3 = a1 * b3;

      uint64_t result[6];
      result[0] = low(a0b0);
      result[1] = low(a0b1) + low(a1b0) + high(a0b0);
      result[2] = low(a0b2) + low(a1b1) + high(a0b1) + high(a1b0);
      result[3] = low(a0b3) + low(a1b2) + high(a0b2) + high(a1b1);
      result[4] = a1b3 + high(a0b3) + high(a1b2);

      result[1] += high(result[0]);
      result[2] += high(result[1]);
      result[3] += high(result[2]);
      result[4] += high(result[3]);
      result[5] = high(result[4]);

      uint64_t ret[4];
      ret[0] = low(result[0]) | (low(result[1]) << 32) + high(result[0]);
      ret[1] = low(result[2]) | low(result[3]) << 32;
      ret[2] = low(result[4]) | low(result[5]) << 32;

      int index = shift_right / 64;
      int shift_right_in_index = shift_right - (index * 64);
      ret[index] >>= shift_right_in_index;
      ret[index] |= (ret[index + 1] & ((uint64_t(1) << shift_right_in_index) - 1)) << (64 - shift_right_in_index);
      return ret[index];
    }
   
    template<>
    inline uint64_t multiply_and_shift<float>(uint64_t a, const uint64_t *b, int shift_right)
    {
      uint64_t a0, a1, b0, b1, a0b0, a0b1, a1b0, a1b1;
      a0 = low(a); a1 = high(a); b0 = low(*b); b1 = high(*b);

      a0b0 = a0 * b0; a0b1 = a0 * b1; a1b0 = a1 * b0; a1b1 = a1 * b1;

      uint64_t result[4] = {};
      result[0] = low(a0b0);
      result[1] = low(a0b1) + low(a1b0) + high(a0b0);
      result[2] = low(a1b1) + high(a0b1) + high(a1b0);
      result[3] = high(a1b1);

      result[1] += high(result[0]);
      result[2] += high(result[1]);
      result[3] += high(result[2]);

      uint64_t ret[4];
      ret[0] = low(result[0]) | (low(result[1]) << 32) + high(result[0]);
      ret[1] = low(result[2]) | low(result[3]) << 32;

      int index = shift_right / 64;
      int shift_right_in_index = shift_right - (index * 64);
      ret[index] >>= shift_right_in_index;
      ret[index] |= (ret[index + 1] & ((uint64_t(1) << shift_right_in_index) - 1)) << (64 - shift_right_in_index);
      return ret[index];
    }

    uint64_t pow_int(int n, int exp)
    {
      if (!exp)
        return 1;
      uint64_t ret = n;
      for (int i = 0; i < exp; i++)
      {
        ret *= ret;
      }
      return ret;
    }

    template<typename T>
    static float_base10 decode(T f)
    {
      bool negative;
      int exp;
      uint64_t mentissa;
      get_parts(f, negative, exp, mentissa);

      if (std::isnan(f))
      {
        return { negative, true };
      }
      if (std::isinf(f))
      {
        return { negative, false, true };
      }
      if (!exp && !mentissa)
      {
        return { negative, false, false, 1, 0, 0 };
      }

      bool accept_larger = (mentissa % 2 == 0);
      bool accept_smaller = accept_larger;

      normalize<T>(exp, mentissa);

      exp -= 2;
      mentissa *= 4;

      uint64_t u = mentissa;
      if (mentissa == 0 && exp > 1)
        u -= 1;
      else
        u -= 2;

      uint64_t w = mentissa + 2;

      int32_t e10 = exp < 0 ? exp : 0;

      int q;
      int shift_right;
      bool zero[3] = {};
      if (exp >= 0)
      {
        q = std::max(0, int(exp * log_10_2) - 1);
        int k = cache_values<T>::b0 + int(q * log_2_5);
        shift_right = -exp + q + k;
        if (q - 1 <= max_double_5_pow_q)
        {
          uint64_t mod = pow_int(5, q - 1);
          zero[1] = (mentissa % mod) == 0;
          if (q <= max_double_5_pow_q)
          {
            mod *= mod;
            zero[0] = (u % mod) == 0;
            zero[2] = (w % mod) == 0;
          }
        }
      }
      else
      {
        q = std::max(0, int(-exp * log_10_5) - 1);
        int k = int(std::ceil((-exp - q) * log_2_5)) - cache_values<T>::b1;
        shift_right = q - k;
        if (q - 1 <= max_double_2_pow_q)
        {
          uint64_t mod = uint64_t(1) << (q - 1);
          zero[1] = (mentissa % mod) == 0;

          if (q <= max_double_2_pow_q)
          {
            mod <<= 1;
            zero[0] = (u % mod) == 0;
            zero[2] = (w % mod) == 0;
          }
        }
      }
      auto cache_value = exp >= 0 ? cache_values<T>::greater_than_equals(q) : cache_values<T>::less_than(-exp - q);
      uint64_t a = multiply_and_shift<T>(u, cache_value, shift_right);
      uint64_t b = multiply_and_shift<T>(mentissa, cache_value, shift_right);
      uint64_t c = multiply_and_shift<T>(w, cache_value, shift_right);

      int32_t exponent_adjust;
      uint64_t shortest_base10;
      compute_shortest(a, b, c, accept_smaller && zero[0], accept_larger || !zero[2], zero[1], exponent_adjust, shortest_base10);
      uint8_t significand_digit_count = digitsInNumber(shortest_base10);
      int32_t e = exponent_adjust + e10 + q + significand_digit_count - 1;
      return { negative, false, false, significand_digit_count, e, shortest_base10 };
    }

    static int to_string_int(const float_base10& result, char* buffer, int32_t buffer_size)
    {
      if (!buffer_size)
        return 0;
      int offset = 0;
      if (result.nan)
      {
        if (buffer_size >= 3)
        {
          buffer[offset++] = 'n'; buffer[offset++] = 'a'; buffer[offset++] = 'n';
        }
        return offset;
      }

      if (result.negative)
      {
        buffer[offset++] = '-';
        buffer_size--;
      }

      if (result.inf)
      {
        if (buffer_size >= 3)
        {
          buffer[offset++] = 'i'; buffer[offset++] = 'n'; buffer[offset++] = 'f';
        }
        return offset;
      }

      char significan_buffer[17];
      assert(result.significand_digit_count <= 17);
      uint64_t significand = result.significand;
      for (int i = 0; i < result.significand_digit_count; i++)
      {
        significan_buffer[result.significand_digit_count - i - 1] = '0' + significand % 10;
        significand /= 10;
      }

      int32_t abs_exp = std::abs(result.exp);
      char exponent_buffer[4];
      int exponent_digit_count = digitsInNumber(uint32_t(abs_exp));
      if (result.exp < 0)
      {
        exponent_buffer[0] = '-';
      }
      for (int i = 0; i < exponent_digit_count; i++)
      {
        exponent_buffer[exponent_digit_count + (result.exp < 0) - i - 1] = '0' + abs_exp % 10;
        abs_exp /= 10;
      }
      exponent_digit_count += result.exp < 0;

      int str_size = result.significand_digit_count + 1 + 1 + exponent_digit_count;
      buffer[offset++] = significan_buffer[0];
      if (buffer_size < 2)
        return 1;
      if (result.significand_digit_count > 1)
        buffer[offset++] = '.';
      if (buffer_size < 3)
        return 2;
      int32_t to_copy = std::min(buffer_size - offset, int32_t(result.significand_digit_count) - 1);
      for (int i = 0; i < to_copy; i++)
      {
        buffer[offset++] = significan_buffer[1 + i];
      }
      if (offset >= buffer_size)
        return offset;

      buffer[offset++] = 'e';
      to_copy = std::min(buffer_size - offset, exponent_digit_count);
      for (int i = 0; i < to_copy; i++)
      {
        buffer[offset++] = exponent_buffer[i];
      }

      return offset;
    }

    inline std::string to_string(float f)
    {
      auto decoded = decode(f);
      std::string ret;
      ret.resize(25);
      ret.resize(to_string_int(decoded, &ret[0], ret.size()));
      return ret;
    }

    inline std::string to_string(double d)
    {
      auto decoded = decode(d);
      std::string ret;
      ret.resize(25);
      ret.resize(to_string_int(decoded, &ret[0], ret.size()));
      return ret;
    }
  }

  struct set_end_ptr
  {
    set_end_ptr(parsed_string& parsedString, const char*& current) : parsedString(parsedString), current(current) {}
    ~set_end_ptr() { parsedString.endptr = current; }
    parsed_string& parsedString;
    const char*& current;
  };

  inline parse_string_error parseNumber(const char* number, size_t size, parsed_string& parsedString)
  {
    const char* current;
    set_end_ptr setendptr(parsedString, current);
    int32_t desimal_position = -1;

    parsedString.negative = false;
    parsedString.inf = 0;
    parsedString.nan = 0;
    parsedString.significand_digit_count = 0;
    parsedString.significand = 0;
    parsedString.exp = 0;

    const char* number_end = number + size;
    current = std::find_if(number, number_end, [](const char a) { return !std::isspace(a); });
    if (number_end == current)
    {
      return parse_string_error::empty_string;
    }
    if (*current == '-')
    {
      parsedString.negative = true;
      current++;
    }
    while (current < number_end)
    {
      if ((*current < '0' || *current > '9') && *current != '.')
        break;

      if (*current == '.')
      {
        if (desimal_position >= 0)
          return parse_string_error::multiple_commas;
        desimal_position = parsedString.significand_digit_count;
      }
      else
      {
        if (parsedString.significand_digit_count < 18)
          parsedString.significand = parsedString.significand * 10 + (*current - '0');
        parsedString.significand_digit_count++;
      }
      current++;
    }
    if (*current != 'e' && *current != 'E')
    {
      if (desimal_position >= 0)
        parsedString.exp = desimal_position - parsedString.significand_digit_count;
      else
        parsedString.exp = 0;
      return parse_string_error::ok;
    }
    current++;
    if (current == number_end)
    {
      return parse_string_error::illegal_exponent_value;
    }
    bool exponent_nagative = false;
    if (*current == '-')
    {
      exponent_nagative = true;
      current++;
    }
    else if (*current == '+')
    {
      current++;
    }
    if (current == number_end)
    {
      return parse_string_error::illegal_exponent_value;
    }
    int exponent = 0;
    bool exponent_assigned = false;
    while (current < number_end)
    {
      if ((*current < '0' || *current > '9'))
        break;
      exponent_assigned = true;
      exponent = exponent * 10 + (*current - '0');
      current++;
    }
    if (!exponent_assigned)
    {
      return parse_string_error::illegal_exponent_value;
    }

    if (exponent_nagative)
      exponent = -exponent;

    if (desimal_position >= 0)
      parsedString.exp = desimal_position - parsedString.significand_digit_count + exponent;
    else
      parsedString.exp = exponent;
    return parse_string_error::ok;
  }

  template<typename T>
  inline T convertToNumber(const parsed_string& parsed)
  {
    int base10exponent = parsed.exp + parsed.significand_digit_count - 1;
    if (base10exponent > float_info<T>::max_base10_exponent())
    {
      return parsed.negative ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity();
    }
    else if (base10exponent < float_info<T>::min_base10_exponent())
    {
      return parsed.negative ? T(-0.0) : T(0.0);
    }
    if (parsed.exp == 0 && parsed.significand == 0)
    {
      return parsed.negative ? T(-0.0) : T(0.0);
    }
    using uint_conversion_type = float_info<T>::str_to_float_conversion_type;
    uint_conversion_type a;
    uint_conversion_type b;
    assign_significand_to_float_conversion_type(parsed, a);
    int desimal_exponent = parsed.exp;
    auto binary_exponent = float_info<T>::str_to_float_binary_exponen_init();
    for (;desimal_exponent > 0; desimal_exponent--)
    {
      left_shift(a);
      copy_conversion_type(a, b);
      left_shift<2>(b);
      add(b, a);

      while (float_info<T>::conversion_type_has_mask(a))
      {
        right_shift(a);
        binary_exponent++;
      }
    }

    for (;desimal_exponent < 0; desimal_exponent++)
    {
      while (!float_info<T>::conversion_type_has_top_bit_in_mask(a))
      {
        left_shift(a);
        binary_exponent--;
      }

      divide_by_10(a);
    }

    if (!float_info<T>::conversion_type_is_null(a))
    {
      while (!float_info<T>::conversion_type_has_mask(a))
      {
        left_shift(a);
        binary_exponent--;
      }
    }

    binary_exponent += float_info<T>::bias();
    T to_digit;
    if (binary_exponent <= 0)
    {
      float_info<T>::copy_denormal_to_type(a, binary_exponent, parsed.negative, to_digit);
    }
    else if (binary_exponent < (int(1) << float_info<T>::exponent_width()) - 2)
    {
      float_info<T>::copy_normal_to_type(a, binary_exponent, parsed.negative, to_digit);
    }
    else
    {
      to_digit = parsed.negative ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity();
    }
    return to_digit;
  }

  inline double to_double(const char* str, size_t size)
  {
    parsed_string ps;
    auto parseResult = parseNumber(str, size, ps);
    if (parseResult != parse_string_error::ok)
      return std::nan("1");
    return convertToNumber<double>(ps);
  }
  
  inline double to_float(const char* str, size_t size)
  {
    parsed_string ps;
    auto parseResult = parseNumber(str, size, ps);
    if (parseResult != parse_string_error::ok)
      return std::nan("1");
    return convertToNumber<float>(ps);
  }
}
#endif //FLOAT_TOOLS_H
