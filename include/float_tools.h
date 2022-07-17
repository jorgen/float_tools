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
#include <string>
#include <type_traits>
#include <cmath>
#include <assert.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace ft
{
  template<typename T>
  struct float_base10
  {
    uint8_t negative;
    uint8_t inf;
    uint8_t nan;
    uint8_t significand_digit_count;
    int exp;
    T significand;
  };

  template<typename T>
  struct parsed_string : float_base10<T>
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
    static_assert(shift < sizeof(*a) * 8, "This functions does only support shifting by sizes smaller than sizeof(*a) * 8");
    a[1] = a[1] << shift | (a[0] >> (int(sizeof(uint64_t) * 8) - shift));
    a[0] = a[0] << shift;
  }

  template<int shift = 1>
  inline void left_shift(uint64_t&a)
  {
    static_assert(shift < sizeof(a) * 8, "This functions does only support shifting by sizes smaller than sizeof(*a) * 8");
    a = a << shift;
  }

  inline void left_shift(uint64_t(&a)[2], int shift)
  {
    if (shift > int(sizeof(*a)) * 8)
    {
      auto shift_0 = (int(sizeof(uint64_t) * 8) - shift);
      if (shift_0 > 0)
        a[1] = a[0] >> shift_0;
      else
        a[1] = a[0] << -shift_0;

      a[0] = 0;
    }
    else
    {
      a[1] = a[1] << shift | (a[0] >> (int(sizeof(uint64_t) * 8) - shift));
      a[0] = a[0] << shift;
    }
  }

  inline void left_shift(uint64_t&a, int shift)
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

  static inline int bit_scan_reverse(uint64_t a)
  {
    assert(a);
#ifdef _MSC_VER
    unsigned long index;
#  ifdef _WIN64
    _BitScanReverse64(&index, a);
#  else
    if  (_BitScanReverse(&index, a >> 32))
      index += 32;
    else
      _BitScanReverse(&index, a & (~uint32_t(0)));
#  endif
    return int(index);
#else
    static_assert (sizeof(unsigned long long ) == sizeof(uint64_t), "Wrong size for builtin_clzll");
    return 63 - __builtin_clzll(a);
#endif
  }

  template<>
  struct float_info<double>
  {
    static inline constexpr int mentissa_width() noexcept { return 52; }
    static inline constexpr int exponent_width() noexcept { return 11; }
    static inline constexpr int bias() noexcept { return (1 << (exponent_width() - 1)) - 1; }
    static inline constexpr int max_base10_exponent() noexcept { return 308; }
    static inline constexpr int min_base10_exponent() noexcept { return -324; }
    static inline constexpr int max_double_5_pow_q() noexcept { return 23; }//floor(log_5(1 << (mentissawidth + 2)))
    static inline constexpr int max_double_2_pow_q() noexcept { return 54; }//floor(log_2(1 << (mentissawidth + 2)))


    using str_to_float_conversion_type = uint64_t[2];
    using uint_alias = uint64_t;
    static inline constexpr int str_to_float_binary_exponent_init() noexcept { return  64 + 60; }
    static inline constexpr uint64_t str_to_float_mask() noexcept { return  ~((uint64_t(1) << 60) - 1); }
    static inline constexpr uint64_t str_to_float_top_bit_in_mask() noexcept { return uint64_t(1) << 63; }
    static inline constexpr int str_to_float_expanded_length() noexcept { return 19; }
    static inline constexpr bool conversion_type_has_mask(const str_to_float_conversion_type& a) noexcept { return a[1] & str_to_float_mask(); }
    static inline constexpr bool conversion_type_has_top_bit_in_mask(const str_to_float_conversion_type& a) noexcept { return a[1] & str_to_float_top_bit_in_mask(); }
    static inline constexpr bool conversion_type_is_null(const str_to_float_conversion_type& a) noexcept { return !a[0] && !a[1]; }
    static inline int shift_left_msb_to_index(str_to_float_conversion_type& a, int index)
    {
      if (a[1])
      {
        int msb = bit_scan_reverse(a[1]);
        int shift_count = index - (msb + 64);
        if (shift_count < 0)
          return 0;
        left_shift(a, shift_count);
        return shift_count;
      }
      else if (a[0])
      {
        int msb = bit_scan_reverse(a[0]);
        int shift_count = index - msb;
        if (shift_count < 0)
          return 0;
        left_shift(a, shift_count);
        return shift_count;
      }
      return 0;
    }
    static inline void copy_denormal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, double& to_digit)
    {
      uint64_t q = a[1];
      int expo_shift = -binary_exponent + 9;
      if (expo_shift)
      {
        q += uint64_t(1) << (expo_shift - 1);
        q >>= expo_shift;
      }
      if (negative)
        q |= uint64_t(1) << 63;
      memcpy(&to_digit, &q, sizeof(q));
    }

    static inline void copy_normal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, double& to_digit)
    {
      uint64_t q = a[1] & ~str_to_float_mask();
      uint64_t to_round_off = (q & ((uint64_t(1) << 8) - 1));
      bool bigger =  to_round_off > (uint64_t(1) << (8 - 1)) || (to_round_off == (uint64_t(1) << (8 - 1)) && a[0]);
      bool tie_odd = (!(q & ((uint64_t(1) << 7) - 1))) && (q & (uint64_t(1)<<8)) && !a[0];
      if (bigger || tie_odd)
      {
        q += uint64_t(1) << (8 - 1);
      }
      q >>= 8;
      q += uint64_t(binary_exponent) << mentissa_width();
      if (negative)
        q |= uint64_t(1) << 63;
      memcpy(&to_digit, &q, sizeof(q));
    }
  };
  
  template<typename T>
  inline void get_parts(T f, bool& negative, int& exp, uint64_t& mentissa)
  {
    uint64_t bits = 0;
    static_assert(sizeof(bits) >= sizeof(f), "Incompatible size");
    memcpy(&bits, &f, sizeof(f));
    exp = int((bits >> float_info<T>::mentissa_width()) & (((uint64_t(1) << float_info<T>::exponent_width()) - 1)));
    mentissa = bits & ((uint64_t(1) << float_info<T>::mentissa_width()) - 1);
    negative = bits >> ((sizeof(f) * 8) - 1);
  }

  template<typename T>
  inline void assign_significand_to_float_conversion_type(const float_base10<T> &significand, uint64_t(&a)[2])
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
    static inline constexpr int max_double_5_pow_q() noexcept { return 10; } //floor(log_5(1 << (mentissawidth + 2)))
    static inline constexpr int max_double_2_pow_q() noexcept { return 25; } //floor(log_2(1 << (mentissawidth + 2)))

    using str_to_float_conversion_type = uint64_t;
    using uint_alias = uint32_t;
    static inline constexpr int str_to_float_binary_exponent_init() noexcept { return 60; }
    static inline constexpr uint64_t str_to_float_mask() noexcept { return  ~((uint64_t(1) << 60) - 1); }
    static inline constexpr uint64_t str_to_float_top_bit_in_mask() noexcept { return uint64_t(1) << 63; }
    static inline constexpr int str_to_float_expanded_length() noexcept { return 10; }
    static inline constexpr bool conversion_type_has_mask(const str_to_float_conversion_type& a) noexcept { return a & str_to_float_mask(); }
    static inline constexpr bool conversion_type_has_top_bit_in_mask(const str_to_float_conversion_type& a) noexcept { return a & str_to_float_top_bit_in_mask(); }
    static inline constexpr bool conversion_type_is_null(const str_to_float_conversion_type& a) noexcept { return !a; }
    static inline int shift_left_msb_to_index(str_to_float_conversion_type& a, int index)
    {
      if (a)
      {
        int msb = bit_scan_reverse(a);
        int shift_count = index - msb;
        if (shift_count < 0)
          return 0;
        left_shift(a, shift_count);
        return shift_count;
      }
      return 0;
    }
    static inline void copy_denormal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, float &to_digit)
    {
      uint64_t q = a;
      int expo_shift = -binary_exponent + 38;
      if (expo_shift)
      {
        q += uint64_t(1) << (expo_shift - 1);
        q >>= expo_shift;
      }
      if (negative)
        q |= uint64_t(1) << 31;
      uint32_t to_copy = uint32_t(q);
      memcpy(&to_digit, &to_copy, sizeof(to_copy));
    }
    static inline void copy_normal_to_type(const str_to_float_conversion_type& a, int binary_exponent, bool negative, float &to_digit)
    {
      uint64_t q = a & ~str_to_float_mask();
      bool bigger = (q & ((uint64_t(1) << 37) - 1)) > (uint64_t(1) << (37 - 1));
      bool tie_odd = (!(q & ((uint64_t(1) << 36) - 1))) && (q & (uint64_t(1)<<37));
      if (bigger || tie_odd)
      {
        q += (uint64_t(1) << (37 - 1));
      }
      q >>= 37;
      q += uint64_t(binary_exponent) << mentissa_width();
      if (negative)
        q |= uint64_t(1) << 31;
      uint32_t to_copy = uint32_t(q);
      memcpy(&to_digit, &to_copy, sizeof(to_digit));
    }
  };

  template<typename T>
  inline T make_zero(bool negative)
  {
    using uint_ft = typename float_info<T>::uint_alias;
    uint_ft tmp = 0;
    tmp = uint_ft(negative) << ((sizeof(T) * 8) - 1);
    T ret;
    memcpy(&ret, &tmp, sizeof(ret));
    return ret;
  }
  
  template<typename T>
  inline T make_inf(bool negative)
  {
    using uint_ft = typename float_info<T>::uint_alias;
    uint_ft tmp = (uint_ft(1) << float_info<T>::exponent_width()) - 1;
    tmp <<= float_info<T>::mentissa_width();
    tmp += uint_ft(negative) << ((sizeof(T) * 8) - 1);
    T ret;
    memcpy(&ret, &tmp, sizeof(ret));
    return ret;
  }

  template<typename T>
  inline T make_nan(bool positive, uint64_t pos = 1)
  {
    if (pos == 0)
      pos++;
    using uint_ft = typename float_info<T>::uint_alias;
    uint_ft tmp = (uint_ft(1) << float_info<T>::exponent_width()) - 1;
    tmp <<= float_info<T>::mentissa_width();
    tmp |= pos;
    tmp |= uint_ft(!positive) << ((sizeof(T) * 8) - 1);
    T ret;
    memcpy(&ret, &tmp, sizeof(ret));
    return ret;
  }

  template<typename T>
  inline bool is_nan(T t)
  {
    bool negative;
    int exp;
    uint64_t mentissa;
    get_parts(t, negative, exp, mentissa);
    return exp == ((int(1) << float_info<T>::exponent_width()) - 1) && mentissa > 0;
  }
  
  template<typename T>
  inline bool is_inf(T t)
  {
    bool negative;
    int exp;
    uint64_t mentissa;
    get_parts(t, negative, exp, mentissa);
    return exp == ((int(1) << float_info<T>::exponent_width()) - 1) && mentissa == 0;
  }

  template<typename T>
  const T& max(const T& a, const T& b)
  {
    return (a < b) ? b : a;
  }
  
  template<typename T>
  const T& min(const T& a, const T& b)
  {
    return (b < a) ? b : a;
  }

  template<typename I, typename P>
  I find_if(I first, I last, P p)
  {
    for (; first != last; ++first) {
      if (p(*first)) {
        return first;
      }
    }
    return last;
  }

  template<typename T>
  inline void assign_significand_to_float_conversion_type(const float_base10<T> &significand, uint64_t &a)
  {
    a = significand.significand;
  }

  inline void copy_conversion_type(const uint64_t &a, uint64_t&b)
  {
    b = a;
  }

  template<typename T, int COUNT, T SUM>
  struct Pow10
  {
    static inline T get() noexcept
    {
      return Pow10<T, COUNT - 1, SUM * T(10)>::get();
    }
  };
  template<typename T, T SUM>
  struct Pow10<T, 1, SUM>
  {
    static inline T get() noexcept
    {
      return SUM;
    }
  };
  template<typename T, T SUM>
  struct Pow10<T, 0, SUM>
  {
    static inline T get() noexcept
    {
      return 1;
    }
  };

  template<typename T, T VALUE, int SUM, T ABORT_VALUE, bool CONTINUE>
  struct StaticLog10
  {
    constexpr static int get() noexcept
    {
      return StaticLog10<T, VALUE / 10, SUM + 1, ABORT_VALUE, VALUE / 10 != ABORT_VALUE>::get();
    }
  };

  template<typename T, T VALUE, T ABORT_VALUE, int SUM>
  struct StaticLog10<T, VALUE, SUM, ABORT_VALUE, false>
  {
    constexpr static int get() noexcept
    {
      return SUM;
    }
  };

  template<typename T, int WIDTH, int CURRENT>
  struct CharsInDigit
  {
    static int lower_bounds(T t) noexcept
    {
      if (Pow10<T, CURRENT + WIDTH / 2, 1>::get() - 1 < t)
      {
        return CharsInDigit<T, WIDTH - (WIDTH / 2 + 1), CURRENT + WIDTH / 2 + 1>::lower_bounds(t);
      }
      return CharsInDigit<T, WIDTH / 2, CURRENT>::lower_bounds(t);
    }
  };
  template<typename T, int CURRENT>
  struct CharsInDigit<T, 0, CURRENT>
  {
    static int lower_bounds(T ) noexcept
    {
      return CURRENT;
    }
  };
  template<typename T, int CURRENT>
  struct CharsInDigit<T, -1, CURRENT>
  {
    static int lower_bounds(T ) noexcept
    {
      return CURRENT;
    }
  };

  template<typename T>
  T iabs(typename std::enable_if<std::is_unsigned<T>::value, T>::type a)
  {
    return a;
  }

  template<typename T>
  T iabs(typename std::enable_if<std::is_signed<T>::value, T>::type a)
  {
    //this
    if (a > 0)
      return a;
    if (a == std::numeric_limits<T>::min())
      a++;
    return -a;
  }

  template<typename T>
  int count_chars(T t) noexcept
  {
    if (iabs<T>(t) < T(10))
      return 1;
    constexpr int maxChars = StaticLog10<T, std::numeric_limits<T>::max(), 0, 0, true>::get() + 1;
    return CharsInDigit<T, maxChars, 0>::lower_bounds(iabs<T>(t)) - 1;
  }

  namespace ryu
  {
#include "cache_ryu.h" //include cache table in namespace

    constexpr static const double log_10_2 = 0.30102999566398114;
    constexpr static const double log_10_5 = 0.6989700043360189;
    constexpr static const double log_2_5 = 2.321928094887362;

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

    inline void compute_shortest(uint64_t a, uint64_t b, uint64_t c, bool accept_smaller, bool accept_larger, bool break_tie_down, int& exponent_adjuster, uint64_t& shortest_base10)
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
      while (a_next < c_next)
      {
        a_remainder = a % 10;
        b_remainder = b % 10;

        all_b_zero &= bool(!b_remainder);
        all_a_zero &= bool(!a_remainder);

        a = a_next;
        b = b_next;
        c = c_next;
        a_next = a / 10;
        b_next = b / 10;
        c_next = c / 10;
        i++;
      }
      if (accept_smaller && all_a_zero && a % 10 == 0)
      {
        while (!(a_next % 10))
        {
          b_remainder = b % 10;
          
          all_b_zero &= bool(!b_remainder);

          a = a_next;
          b = b_next;
          c = c_next;
          a_next = a / 10;
          b_next = b / 10;
          c_next = c / 10;
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

    template<typename T>
    inline uint64_t multiply_and_shift(uint64_t a, const uint64_t* b, int shift_right, bool round_up)
    {
      return 0;
    }
    template<>
    inline uint64_t multiply_and_shift<double>(uint64_t a, const uint64_t *b, int shift_right, bool round_up)
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
      ret[0] = low(result[0]) | ((low(result[1]) << 32) + high(result[0]));
      ret[1] = low(result[2]) | (low(result[3]) << 32);
      ret[2] = low(result[4]) | (low(result[5]) << 32);

      int index = shift_right / 64;
      int shift_right_in_index = shift_right - (index * 64);
      if (round_up)
      {
        if (shift_right_in_index)
        {
          if (!(ret[index] & (uint64_t(1) << (shift_right_in_index - 1))))
            round_up = false;
        }
        else
        {
          if (!(index > 0 && ret[index] & uint64_t(1) << 63))
            round_up = false;
        }
      }
      ret[index] >>= shift_right_in_index;
      ret[index] |= (ret[index + 1] & ((uint64_t(1) << shift_right_in_index) - 1)) << (64 - shift_right_in_index);
      ret[index] += round_up;
      return ret[index];
    }
   
    template<>
    inline uint64_t multiply_and_shift<float>(uint64_t a, const uint64_t *b, int shift_right, bool round_up)
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
      ret[0] = low(result[0]) | ((low(result[1]) << 32) + high(result[0]));
      ret[1] = low(result[2]) | (low(result[3]) << 32);

      int index = shift_right / 64;
      int shift_right_in_index = shift_right - (index * 64);
      if (round_up)
      {
        if (shift_right_in_index)
        {
          if (!(ret[index] & (uint64_t(1) << (shift_right_in_index - 1))))
            round_up = false;
        }
        else
        {
          if (!(index > 0 && ret[index] & uint64_t(1) << 63))
            round_up = false;
        }
      }
      ret[index] >>= shift_right_in_index;
      ret[index] |= (ret[index + 1] & ((uint64_t(1) << shift_right_in_index) - 1)) << (64 - shift_right_in_index);
      ret[index] += round_up;
      return ret[index];
    }

    inline uint64_t pow_int(int n, int exp)
    {
      if (!exp)
        return 1;
      uint64_t ret = uint64_t(n);
      for (int i = 0; i < exp; i++)
      {
        ret *= ret;
      }
      return ret;
    }

    template<typename T, typename SignificandType>
    static float_base10<SignificandType> decode(T f)
    {
      bool negative;
      int exp;
      uint64_t mentissa;
      get_parts(f, negative, exp, mentissa);
      bool shift_u_with_one = mentissa == 0 && exp > 1;

      if (is_nan(f))
      {
        return { negative, false, true, 0, 0 ,0};
      }
      if (is_inf(f))
      {
        return { negative, true, false, 0, 0, 0};
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
      if (shift_u_with_one)
        u -= 1;
      else
        u -= 2;

      uint64_t w = mentissa + 2;

      int e10 = exp < 0 ? exp : 0;

      int q;
      int shift_right;
      bool zero[3] = {};
      if (exp >= 0)
      {
        q = max(0, int(exp * log_10_2) - 1);
        int k = cache_values<T>::b0 + int(q * log_2_5);
        shift_right = -exp + q + k;
        if (q - 1 <= float_info<T>::max_double_5_pow_q())
        {
          uint64_t mod = pow_int(5, q - 1);
          if (mod)
          {
            zero[1] = (mentissa % mod) == 0;
          }
          if (q <= float_info<T>::max_double_5_pow_q())
          {
            mod = pow_int(5, q);
            zero[0] = (u % mod) == 0;
            zero[2] = (w % mod) == 0;
          }
        }
      }
      else
      {
        q = max(0, int(-exp * log_10_5) - 1);
        int k = int(std::ceil((double(-exp) - double(q)) * log_2_5)) - cache_values<T>::b1;
        shift_right = q - k;
        if (q && q - 1 <= float_info<T>::max_double_2_pow_q())
        {
          uint64_t mod = uint64_t(1) << int(q - 1);
          zero[1] = (mentissa % mod) == 0;

          if (q <= float_info<T>::max_double_2_pow_q())
          {
            mod <<= 1;
            if (mod)
            {
              zero[0] = (u % mod) == 0;
              zero[2] = (w % mod) == 0;
            }
          }
        }
      }
      auto cache_value = exp >= 0 ? cache_values<T>::greater_than_equals(q) : cache_values<T>::less_than(-exp - q);
      uint64_t a = multiply_and_shift<T>(u, cache_value, shift_right, true);
      uint64_t b = multiply_and_shift<T>(mentissa, cache_value, shift_right, false);
      uint64_t c = multiply_and_shift<T>(w, cache_value, shift_right, false);

      int exponent_adjust;
      uint64_t shortest_base10;
      compute_shortest(a, b, c, accept_smaller && zero[0], accept_larger || !zero[2], zero[1], exponent_adjust, shortest_base10);
      int significand_digit_count = count_chars(shortest_base10);
      int e = exponent_adjust + e10 + q;
      return { negative, false, false, uint8_t(significand_digit_count), e, shortest_base10 };
    }

    template<typename T>
    inline int convert_parsed_to_buffer(const float_base10<T> &result, char* buffer, int buffer_size, int max_expanded_length, int *digits_truncated = nullptr)
    {
      if (buffer_size < 1)
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

      char significan_buffer[17] = {};
      assert(result.significand_digit_count <= uint8_t(17));
      int digits_before_decimals = result.significand_digit_count + result.exp;
      int digits_after_decimals = result.exp < 0 ? -result.exp : 0;
      int complete_digits = max(1, digits_before_decimals) + max(1, digits_after_decimals) + 1;
      if (complete_digits < max_expanded_length)
      {
        char* target_buffer = buffer + offset;
        uint64_t significand = result.significand;
        bool print_desimal_seperator = true;
        if (buffer_size < complete_digits)
        {
          int to_remove = complete_digits - buffer_size;
          if (digits_truncated)
            *digits_truncated = to_remove;
         
          int to_remove_after_decimals = std::min(to_remove, digits_after_decimals);
          for (int i = 0; i < to_remove_after_decimals; i++)
          {
            complete_digits--;
            digits_after_decimals--;
            significand /= 10;
          }
          to_remove -= to_remove_after_decimals;
          if (to_remove > 0)
          {
            print_desimal_seperator = false;
            if (!digits_after_decimals)
            {
              complete_digits--;
              to_remove--;

            }
            complete_digits--;
            to_remove--;
            if (to_remove > 0)
            {
              int to_remove_before_decimals = std::min(to_remove, digits_before_decimals);
              for (int i = 0; i < to_remove_before_decimals; i++)
              {
                complete_digits--;
                digits_before_decimals--;
                significand /= 10;
              }
            }
          }
          else if (to_remove == 0 && digits_after_decimals == 0)
          {
            print_desimal_seperator = false;
            complete_digits--;
          }
        }
        int index_pos = std::max(complete_digits - 1, 0);
        for (int i = 0; i < digits_after_decimals; i++, index_pos--)
        {
          char remainder = char(significand % 10);
          significand /= 10;
          target_buffer[index_pos] = '0' + remainder;
        }
        if (print_desimal_seperator)
        {
          if (digits_after_decimals == 0)
          {
            target_buffer[index_pos--] = '0';
          }
          target_buffer[index_pos--] = '.';
        }
        int add_zeros_before_decimal = std::max(result.exp, 0);
        for (int i = 0; i < add_zeros_before_decimal; i++, index_pos--)
        {
          target_buffer[index_pos] = '0';
          digits_before_decimals--;
        }
        for (int i = 0; i < digits_before_decimals; i++, index_pos--)
        {
          char remainder = char(significand % 10);
          significand /= 10;
          target_buffer[index_pos] = '0' + remainder;
        }
        if (digits_before_decimals <= 0)
          target_buffer[index_pos] = '0';
        return complete_digits + offset;
      }
      else
      {
        uint64_t significand = result.significand;
        int exp = result.exp;
        for (int i = 0; i < result.significand_digit_count; i++)
        {
          significan_buffer[result.significand_digit_count - i - 1] = '0' + significand % 10;
          significand /= 10;
        }

        exp += result.significand_digit_count;
        exp--;
        char exponent_buffer[4] = {};
        int exponent_digit_count = count_chars(exp);
        if (exp < 0)
        {
          exponent_buffer[0] = '-';
        }
        int abs_exp = std::abs(exp);
        for (int i = 0; i < exponent_digit_count; i++)
        {
          exponent_buffer[exponent_digit_count + (exp < 0) - i - 1] = '0' + abs_exp % 10;
          abs_exp /= 10;
        }
        exponent_digit_count += exp < 0;

        if (offset < buffer_size)
          buffer[offset++] = significan_buffer[0];
        else
          return offset;
        
        if (result.significand_digit_count > 1)
        {
          if (offset < buffer_size)
            buffer[offset++] = '.';
          else return offset;
        }
        int to_copy = min(buffer_size - offset, int(result.significand_digit_count) - 1);
        for (int i = 0; i < to_copy; i++)
        {
          buffer[offset++] = significan_buffer[1 + i];
        }

        if (offset >= buffer_size)
          return offset;

        buffer[offset++] = 'e';

        to_copy = min(buffer_size - offset, exponent_digit_count);
        for (int i = 0; i < to_copy; i++)
        {
          buffer[offset++] = exponent_buffer[i];
        }
      }

      return offset;
    }

  }

  template<typename T>
  struct set_end_ptr
  {
    set_end_ptr(parsed_string<T> &parsedString, const char*& current) : parsedString(parsedString), current(current) {}
    ~set_end_ptr() { parsedString.endptr = current; }
    parsed_string<T> &parsedString;
    const char*& current;
  };

  inline bool is_space(char a)
  {
    if (a == 0x20
      || a == 0x09
      || a == 0x0a
      || a == 0x0b
      || a == 0x0c
      || a == 0x0d
      )
      return true;
    return false;
  }

  template<typename T, bool NoDigitCount>
  inline parse_string_error parseNumber(const char* number, size_t size, parsed_string<T> &parsedString)
  {
    const char* current;
    set_end_ptr<T> setendptr(parsedString, current);
    int desimal_position = -1;
    bool increase_significand = true;

    parsedString.negative = false;
    parsedString.inf = 0;
    parsedString.nan = 0;
    parsedString.significand_digit_count = 0;
    parsedString.significand = 0;
    parsedString.exp = 0;

    const char* number_end = number + size;
    current = find_if(number, number_end, [](const char a) { return !is_space(a); });
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
        if (NoDigitCount || parsedString.significand_digit_count < 19)
        {
          parsedString.significand = parsedString.significand * T(10) + T(int(*current) - '0');
          parsedString.significand_digit_count++;
        }
        else if (increase_significand && parsedString.significand_digit_count < 20)
        {
          increase_significand = false;
          uint64_t digit = uint64_t(*current) - '0';
          static_assert(NoDigitCount || std::is_same<T, uint64_t>::value, "When NoDigitCount is used the significand type has to be uint64_t");
          auto biggest_multiplier = (std::numeric_limits<uint64_t>::max() - digit) / parsedString.significand;

          if (biggest_multiplier >= 10)
          {
            parsedString.significand = parsedString.significand * uint64_t(10) + digit;
            parsedString.significand_digit_count++;
          }
        }
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

  inline uint64_t getPow10(uint32_t pow)
  {
    static uint64_t data[] =
    {
      UINT64_C(1),
      UINT64_C(10),
      UINT64_C(100),
      UINT64_C(1000),
      UINT64_C(10000),
      UINT64_C(100000),
      UINT64_C(1000000),
      UINT64_C(10000000),
      UINT64_C(100000000),
      UINT64_C(1000000000),
      UINT64_C(10000000000),
      UINT64_C(100000000000),
      UINT64_C(1000000000000),
      UINT64_C(10000000000000),
      UINT64_C(100000000000000),
      UINT64_C(1000000000000000),
      UINT64_C(10000000000000000),
      UINT64_C(100000000000000000),
      UINT64_C(1000000000000000000),
      UINT64_C(10000000000000000000)
    };
    return data[pow];
  }

  template<typename T, typename SignificandType>
  inline T convertToNumber(const parsed_string<SignificandType> &parsed)
  {
    int base10exponent = parsed.exp + parsed.significand_digit_count - 1;
    if (base10exponent > float_info<T>::max_base10_exponent())
    {
      return make_inf<T>(parsed.negative);
    }
    else if (base10exponent < float_info<T>::min_base10_exponent())
    {
      return make_zero<T>(parsed.negative);
    }
    if (parsed.significand == 0)
    {
      return make_zero<T>(parsed.negative);
    }

#if 1
    if (parsed.significand < ((uint64_t(1) << 53))
      && iabs<int>(parsed.exp) < count_chars((uint64_t(1) << 53)))
    {
      double ds(double(parsed.significand));
      double de(double(getPow10(iabs<int>(parsed.exp))));
      if (parsed.negative)
        ds = -ds;
      return parsed.exp < 0 ? T(ds / de) : T(ds * de);
    }
#endif

    using uint_conversion_type = typename float_info<T>::str_to_float_conversion_type;
    uint_conversion_type a;
    uint_conversion_type b;
    assign_significand_to_float_conversion_type(parsed, a);
    int desimal_exponent = parsed.exp;
    auto binary_exponent = float_info<T>::str_to_float_binary_exponent_init();
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
      binary_exponent -= float_info<T>::shift_left_msb_to_index(a, float_info<T>::str_to_float_binary_exponent_init());

      divide_by_10(a);
    }

    binary_exponent -= float_info<T>::shift_left_msb_to_index(a, float_info<T>::str_to_float_binary_exponent_init());

    binary_exponent += float_info<T>::bias();
    T to_digit;
    if (binary_exponent <= 0)
    {
      float_info<T>::copy_denormal_to_type(a, binary_exponent, parsed.negative, to_digit);
    }
    else if (binary_exponent < (int(1) << float_info<T>::exponent_width()) - 1)
    {
      float_info<T>::copy_normal_to_type(a, binary_exponent, parsed.negative, to_digit);
    }
    else
    {
      to_digit = make_inf<T>(parsed.negative);
    }
    return to_digit;
  }

  namespace ryu
  {
    template<typename T>
    int to_buffer(T d, char* buffer, int buffer_size, int *digits_truncated = nullptr)
    {
      auto decoded = decode<T, uint64_t>(d);
      return convert_parsed_to_buffer(decoded, buffer, buffer_size, float_info<T>::str_to_float_expanded_length(), digits_truncated);
    }

    template<typename T>
    inline std::string to_string(T f)
    {
      auto decoded = decode<T, uint64_t>(f);
      std::string ret;
      ret.resize(25);
      ret.resize(size_t(convert_parsed_to_buffer(decoded, &ret[0], int(ret.size()), float_info<T>::str_to_float_expanded_length())));
      return ret;
    }
  }

  namespace integer
  {
    template<typename T>
    inline int to_buffer(T integer, char *buffer, int buffer_size, int *digits_truncated = nullptr)
    {
      static_assert(std::is_integral<T>::value, "Tryint to convert non int to string");
      int chars_to_write = ft::count_chars(integer);
      char* target_buffer = buffer;
      bool negative = false;
      if (std::is_signed<T>::value)
      {
        if (integer < 0)
        {
          target_buffer[0] = '-';
          target_buffer++;
          buffer_size--;
          negative = true;
        }
      }
      int to_remove = chars_to_write - buffer_size;
      if (to_remove > 0)
      {
        for (int i = 0; i < to_remove; i++)
        {
          integer /= 10;
        }
        if (digits_truncated)
          *digits_truncated = to_remove;
        chars_to_write -= to_remove;
      }
      else if (digits_truncated)
        *digits_truncated = 0;

      
      for (int i = 0; i < chars_to_write; i++)
      {
        int remainder = integer % 10;
        if (std::is_signed<T>::value)
        {
          if (negative)
            remainder = -remainder;
        }
        integer /= 10;
        target_buffer[chars_to_write - 1 - i] = '0' + char(remainder);
      }
      
      return chars_to_write + negative;
    }

    template<typename T, typename SignificandType>
    inline typename std::enable_if<std::is_signed<T>::value, T>::type make_integer_return_value(SignificandType significand, bool negative)
    {
      return negative ? -T(significand) : T(significand);
    }

    template<typename T, typename SignificandType>
    inline typename std::enable_if<std::is_unsigned<T>::value, T>::type make_integer_return_value(SignificandType significand, bool)
    {
      return T(significand);
    }

    template<typename T, typename SignificandType>
    inline T convert_to_integer(const parsed_string<SignificandType> &parsed)
    {
      if (parsed.inf)
        return parsed.negative ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
      if (parsed.nan)
        return T(0);

      int exp = parsed.exp;
      auto significand = parsed.significand;
      if (exp < 0)
      {
        int chars_in_sig = count_chars(significand);
        if (-exp >= chars_in_sig)
          return T(0);
        while (exp < 0)
        {
          significand /= 10;
          exp++;
        }
      }
      else if (exp > 0)
      {
        int chars_in_sig = count_chars(significand);
        if (exp > ft::StaticLog10<T, std::numeric_limits<T>::max(), 0, 0, true>::get() - chars_in_sig)
          return parsed.negative ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
        while (exp > 0)
        {
          significand *= 10;
          exp--;
        }
      }
      return make_integer_return_value<T>(significand, bool(parsed.negative));
    }

    template<typename T>
    inline parse_string_error to_integer(const char* str, size_t size, T& target, const char* (&endptr))
    {
      using SignificandType = typename std::make_unsigned<T>::type;
      parsed_string<SignificandType> ps;
      auto parseResult = parseNumber<SignificandType,true>(str, size, ps);
      endptr = ps.endptr;
      if (parseResult != parse_string_error::ok)
      {
        target = 0;
      }
      else
      {
        target = convert_to_integer<T>(ps);
      }
      return parseResult;
    }
    
    template<typename T>
    inline parse_string_error to_integer(const std::string &str, T& target, const char* (&endptr))
    {
      return to_integer(str.c_str(), str.size(), target, endptr);
    }
  }
  
  template<typename T>
  inline parse_string_error to_ieee_t(const char* str, size_t size, T &target, const char *(&endptr))
  {
    parsed_string<uint64_t> ps;
    auto parseResult = parseNumber<uint64_t, false>(str, size, ps);
    endptr = ps.endptr;
    if (parseResult != parse_string_error::ok)
    {
      target = make_nan<T>(true, 1);
    }
    else
    {
      target = convertToNumber<T>(ps);
    }
    return parseResult;
  }
 
  inline parse_string_error to_float(const char* str, size_t size, float &target, const char *(&endptr))
  {
    return to_ieee_t(str, size, target, endptr);
  }

  inline parse_string_error to_double(const char* str, size_t size, double &target, const char *(&endptr))
  {
    return to_ieee_t(str, size, target, endptr);
  }

}
#endif //FLOAT_TOOLS_H
