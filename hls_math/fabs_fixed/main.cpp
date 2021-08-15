#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace {

constexpr bool debug = false;

// fabs
template <int W, int I>
ap_fixed<W + 1, I + 1> fabs_fixed(ap_fixed<W, I> x) {
  ap_fixed<W + 1, I + 1> xp = x;
  ap_fixed<W + 1, I + 1> xn = -x;
  return x[W - 1] ? xn : xp;
}

// signbit
template <int W, int I>
ap_uint<1> signbit_fixed(ap_fixed<W, I> x) {
  ap_uint<1> s = x[W - 1];
  return s;
}

template <typename T>
void fabs_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = fabs_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::abs(x_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

template <typename T>
void signbit_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = signbit_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    bool y_bool = std::signbit(x_f32);
    assert(y_bool == static_cast<bool>(y));
  }  // end loop
}

}  // anonymous namespace

// Main function
int main() {
  std::cout << "Test 1 ...";
  typedef ap_fixed<8, 1> T1;
  fabs_fixed_test<T1>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 2 ...";
  typedef ap_fixed<8, 0> T2;
  fabs_fixed_test<T2>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 3 ...";
  typedef ap_fixed<8, 7> T3;
  fabs_fixed_test<T3>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 4 ...";
  typedef ap_fixed<8, -4> T4;
  fabs_fixed_test<T4>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 5 ...";
  typedef ap_fixed<8, 1> T5;
  signbit_fixed_test<T5>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 6 ...";
  typedef ap_fixed<8, 0> T6;
  signbit_fixed_test<T6>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 7 ...";
  typedef ap_fixed<8, 7> T7;
  signbit_fixed_test<T7>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 8 ...";
  typedef ap_fixed<8, -4> T8;
  signbit_fixed_test<T8>();
  std::cout << " OK" << std::endl;

  std::cout << "All OK" << std::endl;
  return 0;
}
