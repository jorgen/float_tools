#include <ttmath/ttmath.h>
#include <fmt/printf.h>

#include <vector>
#include <array>
#include <limits>
#include <algorithm>
#include <cstdint>

using big_uint = ttmath::UInt<32>;
using big_float = ttmath::Big<4, 32>;


namespace ryu
{
  constexpr static const double log_10_2 = 0.30102999566398114;
  constexpr static const double log_10_5 = 0.6989700043360189;
  constexpr static const double log_2_5 = 2.321928094887362;

  template<typename cached_type>
  struct cache_value_t
  {
    int index;
    cached_type data;
  };

  template<typename FLOAT_TYPE>
  struct uint_for_float_converter
  {
  };

  template<>
  struct uint_for_float_converter<float>
  {
    using cache_value_type = cache_value_t<uint64_t>;
    static void assigned_cache_value(cache_value_type& assign_to, const big_uint& value)
    {
      assign_to.data = value.table[0];
    }
    
    static void print_cache_array_declaration(int size)
    {
      fmt::print("    static const uint64_t data[{}] =\n    {{\n", size);
    }
    static void print_data_entry(const cache_value_type& cache_entry)
    {
      fmt::print("      /*{:3}*/ UINT64_C({:20})", cache_entry.index, cache_entry.data);
    }
    
    static void print_return()
    {
      fmt::print("    return &data[index];\n");
    }
  };

  template<>
  struct uint_for_float_converter<double>
  {
    using cache_value_type = cache_value_t<uint64_t[2]>;
    static void assigned_cache_value(cache_value_type &assign_to, const big_uint& value)
    {
      assign_to.data[0] = value.table[0];
      assign_to.data[1] = value.table[1];
    }
    static void print_cache_array_declaration(int size)
    {
      fmt::print("    static const uint64_t data[{}][2] =\n    {{\n", size);
    }
    static void print_data_entry(const cache_value_type& cache_entry)
    {
      fmt::print("      {{/*{:3}*/ UINT64_C({:20}),UINT64_C({:20})}}", cache_entry.index, cache_entry.data[0], cache_entry.data[1]);
    }
    static void print_return()
    {
      fmt::print("    return &data[index][0];\n");
    }

  };

  template<typename T, int mentissa_width, int exponent_width, int b0, int b1>
  struct table_generator
  {
    static int32_t get_exponent(T real)
    {
      uint64_t bits;
      static_assert(sizeof(bits) >= sizeof(real), "Wrong size");
      memcpy(&bits, &real, sizeof(real));
      return int32_t((bits >> mentissa_width) & (((uint64_t(1) << exponent_width) - 1)));
    }

    static int normalize(int exp)
    {
      constexpr static const int bias = (1 << (exponent_width - 1)) - 1;
      if (exp)
      {
        return exp - bias - mentissa_width;
      }
      return 1 - bias - mentissa_width;
    }

    static int get_e2(T real)
    {
      int exp = get_exponent(real);
      exp = normalize(exp);
      return exp - 2;
    }

    static void generate(const std::string float_name)
    {
      static_assert(sizeof(T) <= sizeof(uint64_t), "Wrong size of uint type");

      uint64_t bin_max = (((uint64_t(1) << exponent_width) - 2) << mentissa_width) | ((uint64_t(1) << mentissa_width) - 1);
      T max;
      memcpy(&max, &bin_max, sizeof(max));
     
      uint64_t bin_min = 1;
      T min;
      memcpy(&min, &bin_min, sizeof(min));

      //int t_min_e2 = get_e2(min);
      //int t_max_e2 = get_e2(max);
      int t_min_e2 = get_e2(std::numeric_limits<T>::min());
      int t_max_e2 = get_e2(std::numeric_limits<T>::max());

      using cache_value_type = typename uint_for_float_converter<T>::cache_value_type;
      std::vector<cache_value_type> gte;
      gte.reserve(t_max_e2);

      int generated_b0 = 0;
      int generated_b1 = 0;
      for (int i = 0; i < t_max_e2; i++)
      {
        int q = std::max(0, int(i * log_10_2) - 1);
        auto it = std::find_if(gte.begin(), gte.end(), [q](const cache_value_type &cv) { return cv.index == q; });
        if (it != gte.end())
          continue;
        int k = int(b0 + q * log_2_5);
        big_uint cache_value = 2;
        cache_value.Pow(k);
        big_uint div(5);
        div.Pow(q);
        cache_value.Div(div);
        uint64_t top_table_index;
        uint64_t top_bit_in_index;
        cache_value.FindLeadingBit(top_table_index, top_bit_in_index);
        assert(top_table_index < 2);
        int top_bit = int(64 * top_table_index + top_bit_in_index);
        if (generated_b0 < top_bit)
          generated_b0 = top_bit;
        gte.emplace_back();
        auto& value = gte.back();
        value.index = q;
        uint_for_float_converter<T>::assigned_cache_value(value, cache_value);
      }

      std::vector<cache_value_type> lt;
      lt.reserve(-t_min_e2);
      for (int i = 0; i < -t_min_e2; i++)
      {
        int q = std::max(0, int(i * log_10_5) - 1);
        auto it = std::find_if(lt.begin(), lt.end(), [i, q](const cache_value_type &cv) { return cv.index == (i - q); });
        if (it != lt.end())
          continue;
        int k = int(std::ceil((double(i) - q) * log_2_5) - b1);
        big_uint cache_value = 5;
        cache_value.Pow(i - q);
        if (k > 0)
          cache_value >>= k;
        else
          cache_value <<= -k;
        uint64_t top_table_index;
        uint64_t top_bit_in_index;
        cache_value.FindLeadingBit(top_table_index, top_bit_in_index);
        assert(top_table_index < 2);
        int top_bit = int(64 * top_table_index + top_bit_in_index);
        if (generated_b1 < top_bit)
          generated_b1 = top_bit;
        lt.emplace_back();
        auto& value = lt.back();
        value.index = i - q;
        uint_for_float_converter<T>::assigned_cache_value(value, cache_value);
      }

      assert(b0 == generated_b0);
      assert(b1 == generated_b1);

      fmt::print("template<>\n");
      fmt::print("struct cache_values<{}>\n{{\n", float_name);
      fmt::print("  constexpr static const int b0 = {};\n", b0);
      fmt::print("  constexpr static const int b1 = {};\n\n", b1);
      fmt::print("  static const uint64_t *less_than(int index)\n  {{\n");
      uint_for_float_converter<T>::print_cache_array_declaration(lt.size());
      for (int i = 0; i < lt.size(); i++)
      {
        uint_for_float_converter<T>::print_data_entry(lt[i]);
        if (i + 1 < lt.size())
        {
          i++;
          fmt::print(",");
          uint_for_float_converter<T>::print_data_entry(lt[i]);
          if (i + 1 < lt.size())
            fmt::print(",\n");
          else
            fmt::print("\n");
        }
        else
        {
          fmt::print("\n");
        }
      }
      fmt::print("    }};\n");
      uint_for_float_converter<T>::print_return();
      fmt::print("  }}\n\n");
      fmt::print("  static const uint64_t *greater_than_equals(int index)\n  {{\n");
      uint_for_float_converter<T>::print_cache_array_declaration(gte.size());
      for (int i = 0; i < gte.size(); i++)
      {
        uint_for_float_converter<T>::print_data_entry(gte[i]);
        if (i + 1 < gte.size())
        {
          i++;
          fmt::print(",");
          uint_for_float_converter<T>::print_data_entry(gte[i]);
          if (i + 1 < gte.size())
            fmt::print(",\n");
          else
            fmt::print("\n");
        }
        else
        {
          fmt::print("\n");
        }
      }
      fmt::print("    }};\n");
      uint_for_float_converter<T>::print_return();
      fmt::print("  }}\n");
      fmt::print("}};\n");
    }
  };

}

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  fmt::print("template<typename T>\nstruct cache_values\n{{\n}};\n\n");

  constexpr static const int double_mentissa_width = 52;
  constexpr static const int double_exponent_width = 11;
  constexpr static const int double_b0 = 124;
  constexpr static const int double_b1 = 124;
  ryu::table_generator<double, double_mentissa_width, double_exponent_width, double_b0, double_b1>::generate("double");

  fmt::print("\n");

  constexpr static const int float_mentissa_width = 23;
  constexpr static const int float_exponent_width = 8;
  constexpr static const int float_b0 = 59;
  constexpr static const int float_b1 = 61;
  ryu::table_generator<float, float_mentissa_width, float_exponent_width, float_b0, float_b1>::generate("float");
  fmt::print("\n");
	return 0;
}
