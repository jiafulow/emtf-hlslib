#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace {

constexpr bool debug = false;

template <typename T>
struct make_recip_return_type {};

template <int W, int I>
struct make_recip_return_type<ap_fixed<W, I> > {
  constexpr static const int _W1 = 2;  // require at least 2 integer bits to represent +1.
  constexpr static const int _I1 = 2;
  constexpr static const int _F1 = 0;
  constexpr static const bool _S1 = true;
  constexpr static const int _W2 = W;
  constexpr static const int _I2 = I;
  constexpr static const int _F2 = W - I;
  constexpr static const bool _S2 = true;
  typedef ap_fixed<_W1 + AP_MAX(_I2, 0) + AP_MAX(_F2, 0) + (_S2 && !_S1), _I1 + _F2 + (_S2 && !_S1)> type;
};

// recip
template <int W, int I>
typename make_recip_return_type<ap_fixed<W, I> >::type recip_fixed(ap_fixed<W, I> x) {
  typedef typename make_recip_return_type<ap_fixed<W, I> >::type return_type;
  if (x == 0)
    return 0;
  // Use ap_fixed<2 + F, 2> to represent the dividend, where F is set to the
  // num of integer bits of the divisor
  ap_fixed<2 + AP_MAX(I, 0), 2> one = 1.0f;
  return_type ret = one / x;
  return ret;
}

template <typename T, typename R = typename make_recip_return_type<T>::type>
void recip_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = recip_fixed(x);
    if (debug)
      std::cout << x << " " << y << std::endl;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_f32 = (x_f32 != 0.0f) ? (1.0f / x_f32) : 0.0f;
    float q_f32 = std::ldexp(1.0f, -(R::width - R::iwidth));
    y_f32 = q_f32 * std::trunc(y_f32 / q_f32);  // round toward zero
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

}  // anonymous namespace

// Main function
int main() {
  std::cout << "Test 1 ...";
  typedef ap_fixed<8, 1> T1;
  recip_fixed_test<T1>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 2 ...";
  typedef ap_fixed<8, 0> T2;
  recip_fixed_test<T2>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 3 ...";
  typedef ap_fixed<8, 7> T3;
  recip_fixed_test<T3>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 4 ...";
  typedef ap_fixed<8, -4> T4;
  recip_fixed_test<T4>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 5 ...";
  typedef ap_fixed<8, 8> T5;
  recip_fixed_test<T5>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 6 ...";
  typedef ap_fixed<8, -8> T6;
  recip_fixed_test<T6>();
  std::cout << " OK" << std::endl;

  std::cout << "All OK" << std::endl;
  return 0;
}
