#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace {

constexpr bool debug = false;

// fract
template <int W, int I>
ap_fixed<W - I + 1, 1> fract_fixed(ap_fixed<W, I> x) {
  if (W == I) {
    return 0;
  } else {
    ap_fixed<W - I + 1, 1> xp = x;
    ap_fixed<W - I + 1, 1> xn = x + 1;
    return x[W - 1] ? xn : xp;
  }
}

template <typename T>
void fract_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = fract_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::remainder(x_f32, 1.0f);
    if (std::signbit(y_f32) and (y_f32 != 0))
      y_f32 += 1.0f;
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

}  // anonymous namespace

// Main function
int main() {
  std::cout << "Test 1 ...";
  typedef ap_fixed<4, 1> T1;
  fract_fixed_test<T1>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 2 ...";
  typedef ap_fixed<4, 0> T2;
  fract_fixed_test<T2>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 3 ...";
  typedef ap_fixed<4, -1> T3;
  fract_fixed_test<T3>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 4 ...";
  typedef ap_fixed<4, -2> T4;
  fract_fixed_test<T4>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 5 ...";
  typedef ap_fixed<4, 4> T5;
  fract_fixed_test<T5>();
  std::cout << " OK" << std::endl;

  std::cout << "All OK" << std::endl;
  return 0;
}
