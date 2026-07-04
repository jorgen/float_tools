#include <float_tools.h>

#include "catch.hpp"

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <limits>

namespace
{
  uint32_t float_bits(float f) { uint32_t b; memcpy(&b, &f, sizeof(b)); return b; }
  uint64_t double_bits(double d) { uint64_t b; memcpy(&b, &d, sizeof(b)); return b; }

  float parse_float(const char* s) { float v; const char* e; ft::to_float(s, strlen(s), v, e); return v; }
  double parse_double(const char* s) { double v; const char* e; ft::to_double(s, strlen(s), v, e); return v; }

  // Independent arbitrary-precision oracle for the correctly rounded (round to
  // nearest, ties to even) float value of a decimal string. Deliberately does
  // not share code with the library so it can catch regressions in it.
  struct Big
  {
    std::vector<uint32_t> w{0};
    void norm() { while (w.size() > 1 && w.back() == 0) w.pop_back(); }
    void mul(uint32_t m) { uint64_t c = 0; for (auto& x : w) { uint64_t p = uint64_t(x) * m + c; x = uint32_t(p); c = p >> 32; } if (c) w.push_back(uint32_t(c)); }
    void add(uint32_t a) { uint64_t c = a; for (size_t i = 0; i < w.size() && c; i++) { uint64_t p = uint64_t(w[i]) + c; w[i] = uint32_t(p); c = p >> 32; } if (c) w.push_back(uint32_t(c)); }
    void mul_pow5(int n) { for (int i = 0; i < n; i++) mul(5); }
    void shl(int bits)
    {
      int words = bits / 32, b = bits % 32;
      if (b) { uint32_t c = 0; for (auto& x : w) { uint64_t p = (uint64_t(x) << b) | c; x = uint32_t(p); c = uint32_t(p >> 32); } if (c) w.push_back(c); }
      if (words) w.insert(w.begin(), size_t(words), 0u);
    }
  };
  int cmp(Big a, Big b)
  {
    a.norm(); b.norm();
    if (a.w.size() != b.w.size()) return a.w.size() < b.w.size() ? -1 : 1;
    for (size_t i = a.w.size(); i-- > 0;) if (a.w[i] != b.w[i]) return a.w[i] < b.w[i] ? -1 : 1;
    return 0;
  }
  // sign(value_digits * 10^dexp  -  mid_int * 2^mid_pow2)
  int cmp_value_mid(Big value_digits, int dexp, Big mid_int, int mid_pow2)
  {
    int min5 = dexp < 0 ? dexp : 0;
    int min2 = dexp < mid_pow2 ? dexp : mid_pow2;
    Big l = value_digits; l.mul_pow5(dexp - min5); l.shl(dexp - min2);
    Big r = mid_int;       r.mul_pow5(0 - min5);   r.shl(mid_pow2 - min2);
    return cmp(l, r);
  }
  void float_as_int_pow2(float g, Big& m, int& e2)
  {
    uint32_t b = float_bits(g), ce = (b >> 23) & 0xff, mant = b & 0x7fffff;
    m = Big(); m.w[0] = ce ? (mant | 0x800000) : mant; e2 = ce ? int(ce) - 150 : -149;
  }
  void pair_midpoint(float g1, float g2, Big& mid, int& e2)
  {
    Big m1, m2; int e1, ee2; float_as_int_pow2(g1, m1, e1); float_as_int_pow2(g2, m2, ee2);
    int emin = e1 < ee2 ? e1 : ee2; m1.shl(e1 - emin); m2.shl(ee2 - emin);
    size_t n = m1.w.size() > m2.w.size() ? m1.w.size() : m2.w.size(); m1.w.resize(n, 0);
    uint64_t c = 0; for (size_t i = 0; i < n; i++) { uint64_t p = uint64_t(m1.w[i]) + (i < m2.w.size() ? m2.w[i] : 0) + c; m1.w[i] = uint32_t(p); c = p >> 32; }
    if (c) m1.w.push_back(uint32_t(c)); mid = m1; e2 = emin - 1;
  }
  bool even_mantissa(float g) { return (float_bits(g) & 1) == 0; }

  float oracle_float(const char* str)
  {
    float c = strtof(str, nullptr);
    if (!std::isfinite(c) || c == 0.0f) return c;
    bool neg = c < 0.0f;
    float af = neg ? -c : c;
    const char* p = str;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '-' || *p == '+') p++;
    Big digits; int frac = 0; bool dot = false;
    for (; *p; ++p) { char ch = *p; if (ch == '.') { if (dot) break; dot = true; continue; } if (ch < '0' || ch > '9') break; digits.mul(10); digits.add(uint32_t(ch - '0')); if (dot) frac++; }
    int dexp = -frac;
    if (*p == 'e' || *p == 'E') { p++; bool en = false; if (*p == '-' || *p == '+') { en = (*p == '-'); p++; } int ex = 0; while (*p >= '0' && *p <= '9') { ex = ex * 10 + (*p - '0'); p++; } dexp += en ? -ex : ex; }
    float up = std::nextafter(af, HUGE_VALF), dn = std::nextafter(af, 0.0f);
    float mag = af;
    if (std::isfinite(up) && up > af) { Big mid; int e2; pair_midpoint(af, up, mid, e2); int r = cmp_value_mid(digits, dexp, mid, e2); if (r > 0) mag = up; else if (r == 0) mag = even_mantissa(af) ? af : up; }
    if (mag == af) { Big mid; int e2; pair_midpoint(dn, af, mid, e2); int r = cmp_value_mid(digits, dexp, mid, e2); if (r < 0) mag = dn; else if (r == 0) mag = even_mantissa(dn) ? dn : af; }
    return neg ? -mag : mag;
  }
}

TEST_CASE("string to float is correctly rounded near float midpoints", "[roundtrip][float]")
{
  std::mt19937 g(0xA11CEu);
  uint64_t checked = 0;
  for (int i = 0; i < 150000; i++)
  {
    uint32_t u = g();
    float f; memcpy(&f, &u, sizeof(f));
    if (!std::isfinite(f) || f <= 0.0f) continue;
    float fu = std::nextafterf(f, HUGE_VALF);
    if (!std::isfinite(fu)) continue;
    double m = 0.5 * (double(f) + double(fu));
    double vals[3] = { std::nextafter(m, -HUGE_VAL), m, std::nextafter(m, HUGE_VAL) };
    for (double v : vals)
    {
      char buf[64];
      snprintf(buf, sizeof(buf), "%.25e", v);
      float got = parse_float(buf), want = oracle_float(buf);
      if (float_bits(got) != float_bits(want)) { INFO(buf); REQUIRE(float_bits(got) == float_bits(want)); }
      checked++;
    }
  }
  REQUIRE(checked > 0);
}

TEST_CASE("string to float regression cases", "[roundtrip][float]")
{
  // Double-rounding case: parsing via double then narrowing gives 0x41000000,
  // but the correctly rounded float is 0x41000001.
  REQUIRE(float_bits(parse_float("8000000476837159e-15")) == 0x41000001u);
  // Exact float midpoints must round to even, not away (regression vs some strtof).
  REQUIRE(float_bits(parse_float("61702.845703125")) == float_bits(oracle_float("61702.845703125")));
  REQUIRE(float_bits(parse_float("3.0225367059766915891200000e+21")) == float_bits(oracle_float("3.0225367059766915891200000e+21")));
  REQUIRE(parse_float("0.12345") == 0.12345f);
}

TEST_CASE("string to double matches strtod", "[roundtrip][double]")
{
  std::mt19937_64 g(0x1234567890abcdefULL);
  for (int i = 0; i < 500000; i++)
  {
    int ndig = 1 + int(g() % 17);
    std::string s;
    if (g() & 1) s += '-';
    s += char('1' + g() % 9);
    for (int k = 1; k < ndig; k++) s += char('0' + g() % 10);
    int ex = int(g() % 640) - 320;
    s += 'e'; s += std::to_string(ex);
    double a = parse_double(s.c_str());
    double b = strtod(s.c_str(), nullptr);
    if (double_bits(a) != double_bits(b)) { INFO(s); REQUIRE(double_bits(a) == double_bits(b)); }
  }
}

TEST_CASE("to_integer clamps out of range values", "[integer]")
{
  const char* e;
  int32_t v;
  ft::integer::to_integer<int32_t>("-2147483648", 11, v, e); REQUIRE(v == std::numeric_limits<int32_t>::min());
  ft::integer::to_integer<int32_t>("2147483647", 10, v, e);  REQUIRE(v == std::numeric_limits<int32_t>::max());
  ft::integer::to_integer<int32_t>("3000000000", 10, v, e);  REQUIRE(v == std::numeric_limits<int32_t>::max());
  ft::integer::to_integer<int32_t>("-3000000000", 11, v, e); REQUIRE(v == std::numeric_limits<int32_t>::min());
  ft::integer::to_integer<int32_t>("-42", 3, v, e);          REQUIRE(v == -42);
  uint32_t u;
  ft::integer::to_integer<uint32_t>("5000000000", 10, u, e); REQUIRE(u == std::numeric_limits<uint32_t>::max());
  int64_t w;
  ft::integer::to_integer<int64_t>("-9223372036854775808", 20, w, e); REQUIRE(w == std::numeric_limits<int64_t>::min());
  ft::integer::to_integer<int64_t>("9223372036854775807", 19, w, e);  REQUIRE(w == std::numeric_limits<int64_t>::max());
}

TEST_CASE("to_double handles short and exponent-less inputs", "[parse]")
{
  REQUIRE(parse_double("123") == 123.0);
  REQUIRE(parse_double("0.004") == 0.004);
  // size-bounded, non null-terminated buffers must not read past the end
  char* p = (char*)malloc(3); memcpy(p, "123", 3);
  double d; const char* e; auto r = ft::to_double(p, 3, d, e);
  REQUIRE(r == ft::parse_string_error::ok); REQUIRE(d == 123.0);
  free(p);
  char* dash = (char*)malloc(1); dash[0] = '-';
  double d2; ft::to_double(dash, 1, d2, e); // just must not over-read
  free(dash);
}

TEST_CASE("ryu::to_buffer stays within a small buffer", "[buffer]")
{
  const double vals[] = { 1e14, 12300.0, 987654321.0, 1234.5678, 1e300, 42.0, 0.00012345 };
  for (double v : vals)
    for (int bsz = 1; bsz <= 8; bsz++)
    {
      std::vector<char> buf(size_t(bsz), '\0');
      int trunc = 0;
      int n = ft::ryu::to_buffer<double>(v, buf.data(), bsz, &trunc);
      REQUIRE(n <= bsz);
      REQUIRE(n >= 0);
    }
}
