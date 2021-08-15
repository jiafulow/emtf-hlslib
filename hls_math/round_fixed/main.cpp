#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace {

constexpr bool debug = false;

// round away from zero
template <int W, int I>
ap_fixed<W + 1, I + 1> round_fixed(ap_fixed<W, I> x) {
  if (W == I) {
    return x;
  } else if (I < 0) {
    return 0;
  } else {
    ap_ufixed<W, I> x_abs = 0;
    if (x[W - 1]) {
      x_abs = -x;
    } else {
      x_abs = x;
    }
    const ap_ufixed<1, 0> half_val = 0.5;
    ap_ufixed<W + 1, I + 1> r = x_abs + half_val;
    r(W - I - 1, 0) = 0;
    if (x[W - 1]) {
      return -r;
    } else {
      return r;
    }
  }
}

// floor
// require at least 1 integer bit to represent -1.
template <int W, int I>
ap_fixed<AP_MAX(I, 1) + W - I, AP_MAX(I, 1)> floor_fixed(ap_fixed<W, I> x) {
  if (W == I) {
    return x;
  } else if (I < 0) {
    if (x[W - 1]) {
      return -1;
    } else {
      return 0;
    }
  } else {
    ap_fixed<W + 1, I + 1> r = x;
    r(W - I - 1, 0) = 0;
    return r;
  }
}

// ceil
// require at least 2 integer bits to represent +1.
template <int W, int I>
ap_fixed<AP_MAX(I, 2) + W - I, AP_MAX(I, 2)> ceil_fixed(ap_fixed<W, I> x) {
  if (W == I) {
    return x;
  } else if (I < 0) {
    if (x[W - 1] or x == 0) {
      return 0;
    } else {
      return 1;
    }
  } else {
    ap_fixed<W + 2, I + 2> r = x;
    if (r(W - I - 1, 0) != 0) {
      r(W - I - 1, 0) = 0;
      r += 1;
    }
    return r;
  }
}

// trunc
template <int W, int I>
ap_fixed<W, I> trunc_fixed(ap_fixed<W, I> x) {
  if (W == I) {
    return x;
  } else if (I < 0) {
    return 0;
  } else {
    if (x(W - I - 1, 0) != 0) {
      x(W - I - 1, 0) = 0;
      if (x[W - 1]) {
        x += 1;
      }
    }
    return x;
  }
}

// rint
template <int W, int I>
ap_fixed<W + 1, I + 1> rint_fixed(ap_fixed<W, I> x) {
  if (W == I) {
    return x;
  } else if (I < 0) {
    return 0;
  } else {
    ap_ufixed<W, I> x_abs = 0;
    if (x[W - 1]) {
      x_abs = -x;
    } else {
      x_abs = x;
    }
    const ap_ufixed<1, 0> half_val = 0.5;
    ap_ufixed<W + 1, I + 1> r = x_abs + half_val;
    if (r(W - I - 1, 0) == 0) {
      r[W - I] = 0;  // round to even
    } else {
      r(W - I - 1, 0) = 0;
    }
    if (x[W - 1]) {
      return -r;
    } else {
      return r;
    }
  }
}

template <typename T>
void round_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = round_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::round(x_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

template <typename T>
void floor_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = floor_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::floor(x_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

template <typename T>
void ceil_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = ceil_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::ceil(x_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

template <typename T>
void trunc_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = trunc_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::trunc(x_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

template <typename T>
void rint_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = rint_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::rint(x_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

}  // anonymous namespace

// Main function
int main() {
  std::cout << "Test 1 ...";
  typedef ap_fixed<4, 1> T1;
  round_fixed_test<T1>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 2 ...";
  typedef ap_fixed<4, 0> T2;
  round_fixed_test<T2>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 3 ...";
  typedef ap_fixed<4, -1> T3;
  round_fixed_test<T3>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 4 ...";
  typedef ap_fixed<4, -2> T4;
  round_fixed_test<T4>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 5 ...";
  typedef ap_fixed<4, 4> T5;
  round_fixed_test<T5>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 6 ...";
  typedef ap_fixed<4, 1> T6;
  floor_fixed_test<T6>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 7 ...";
  typedef ap_fixed<4, 0> T7;
  floor_fixed_test<T7>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 8 ...";
  typedef ap_fixed<4, -1> T8;
  floor_fixed_test<T8>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 9 ...";
  typedef ap_fixed<4, -2> T9;
  floor_fixed_test<T9>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 10 ...";
  typedef ap_fixed<4, 4> T10;
  floor_fixed_test<T10>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 11 ...";
  typedef ap_fixed<4, 1> T11;
  ceil_fixed_test<T11>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 12 ...";
  typedef ap_fixed<4, 0> T12;
  ceil_fixed_test<T12>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 13 ...";
  typedef ap_fixed<4, -1> T13;
  ceil_fixed_test<T13>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 14 ...";
  typedef ap_fixed<4, -2> T14;
  ceil_fixed_test<T14>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 15 ...";
  typedef ap_fixed<4, 4> T15;
  ceil_fixed_test<T15>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 16 ...";
  typedef ap_fixed<4, 1> T16;
  trunc_fixed_test<T16>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 17 ...";
  typedef ap_fixed<4, 0> T17;
  trunc_fixed_test<T17>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 18 ...";
  typedef ap_fixed<4, -1> T18;
  trunc_fixed_test<T18>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 19 ...";
  typedef ap_fixed<4, -2> T19;
  trunc_fixed_test<T19>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 20 ...";
  typedef ap_fixed<4, 4> T20;
  trunc_fixed_test<T20>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 21 ...";
  typedef ap_fixed<4, 1> T21;
  rint_fixed_test<T21>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 22 ...";
  typedef ap_fixed<4, 0> T22;
  rint_fixed_test<T22>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 23 ...";
  typedef ap_fixed<4, -1> T23;
  rint_fixed_test<T23>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 24 ...";
  typedef ap_fixed<4, -2> T24;
  rint_fixed_test<T24>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 25 ...";
  typedef ap_fixed<4, 4> T25;
  rint_fixed_test<T25>();
  std::cout << " OK" << std::endl;

  std::cout << "All OK" << std::endl;
  return 0;
}
